# to_pixels: a small antialiased UI library for pixel buffers

## Intent

Small audio devices (sndstation handhelds, embedded hardware hosts) show a
plugin's UI on a 640x480-ish screen and drive it with a d-pad and buttons, not a
mouse. The clap-wrapper embedded-hardware extension already defines how a plugin
draws there: `pixels/1` (`hw_extensions/include/clapwrapper/embedded_hardware.h`).
The host hands the plugin an XRGB8888 buffer with a stride inside
`display_render()`, only after the plugin calls `request_frame()`, and says through
`buffer_age` whether the buffer still holds an earlier frame.

to_pixels is the library a plugin uses to draw that UI. The driving case is
OB-Xf: it should be able to take to_pixels as a dependency and grow a separate
small-devices UI without pulling in SDL, JUCE-on-device, or anything else heavy.
"Small dependency set" is the point of the project, not a nice-to-have.

Existing options were considered and rejected: LVGL (right features, but ~100k
lines, its own object model, styles, allocator and mostly global state, which is
wrong for several plugin instances in one process); microui (tiny, but no
rasteriser, font, knob or antialiasing); Nuklear rawfb (large header, dated look);
canvas_ity / olive.c (rasterisers only).

## Goals

- Draw antialiased, good-looking UI into a buffer the library does not own.
- A persistent, small component tree with d-pad focus and partial redraw.
- First widgets: a **knob**, a **slider**, a **preset list**, plus a **label**.
- Event-driven: idle UI costs nothing; a change marks only its area dirty.
- Map directly onto `pixels/1`: `request_frame()` when something is dirty,
  draw on `display_render()`, honour `buffer_age`.
- Safe for many instances in one process: no globals, no singletons.
- Dependencies: C++20 standard library and vendored `stb_truetype.h`. Tests may
  additionally use `stb_image_write.h` and Catch2.

## Non-goals (this round)

- Porting Bacon Beats' screens. Bacon Beats keeps using SDL; to_pixels does not
  need its offscreen layers, additive glow, rotated glyphs or triangle meshes yet.
- Pointer / touch / mouse input. The design leaves room (components have bounds)
  but no hit testing or drag is built.
- A layout engine. The app sets bounds in pixels.
- Windowing, event loops, or any platform layer inside the library.
- Animation. Value and focus changes repaint immediately. `Root::tick(dt)` exists
  for key repeat and leaves room for animation later.
- Image loading (no stb_image), gradients beyond a simple vertical two-stop fill,
  arbitrary path filling.

## Anti-goals

- Growing a dependency OB-Xf would object to: SDL, freetype, harfbuzz, a GL
  context, a platform SDK, or a JSON library in the core target.
- Full redraws on every change. On device that is the CPU cost we are avoiding.
- Global or static mutable state anywhere in the library.
- Looking like a debug UI: jagged edges, bitmap fonts, unaligned text.

## Constraints

- C++20, CMake >= 3.21, static library target `to_pixels`, alias
  `to_pixels::to_pixels`. Must build with `add_subdirectory` from a parent project
  (OB-Xf, bacon_beats style) with PIC and hidden visibility friendly code.
- Builds cleanly on macOS (clang), Linux arm64/x86_64 (gcc, clang) and Windows
  (MSVC), since OB-Xf ships on all three and will add_subdirectory it everywhere
  (builder's call, not Paul's). No SIMD requirement; plain scalar code that the
  compiler can vectorise is fine.
- Performance (Paul): the UI must hold 30 fps on the RG40XX H (Allwinner H700,
  4x Cortex-A53), so a full redraw of the OB-Xf page fits in 33 ms there. The
  builder adds a target for the common case: redrawing one changed knob or
  slider in under 2 ms. A `to_pixels_bench` executable (no extra deps) times
  both and prints the numbers; running it on the device and hitting them is part
  of done. Use the fast paths the budget needs (solid spans, no per-pixel
  `atan2` where an angle test against the arc's end vectors will do, span-wise
  glyph blits).
- Pixel format is XRGB8888 (`0x00RRGGBB` in a `uint32_t`, as `pixels/1` says),
  arbitrary `strideBytes`. The X byte is written as 0xFF (opaque) and never read.
- Include guards are `#ifndef`/`#define`, not `#pragma once`. Comments short,
  one line, why not what. Code is formatted with a `.clang-format` added to the
  repo (copy the surge-synthesizer style used in sibling projects).

## Approaches considered

**A. Persistent component tree (picked).** Components with bounds, children,
`paint(Canvas&)` and key handling; a root that tracks dirty rects and focus.
Picked because a partial redraw maps straight onto `buffer_age` and
`request_frame()`, full redraws are too expensive on device, and it matches how
OB-Xf's JUCE UI already thinks about controls. Paul is fine maintaining a small
tree.

**B. Immediate mode (microui style).** Rebuild the UI every frame from calls like
`ui.knob(id, v)`. Less code and no tree to keep in sync, but every frame redraws
everything, `buffer_age` is wasted, and focus/animation state ends up keyed by id
anyway. Rejected.

In both, the drawing layer is a standalone `Canvas` usable without widgets.

## Design

### Layer 1: Surface and Canvas

- `Surface`: non-owning view `{uint32_t *pixels; int width, height, strideBytes;}`.
  `Image`: an owning surface (a `std::vector<uint32_t>`), used by tests and demos.
- `Color`: straight (non-premultiplied) RGBA, 8 bits per channel, with helpers
  `withAlpha`, `mix`. Blending is source-over onto the opaque destination, done in
  sRGB space (what every UI toolkit the user knows does).
- `Canvas(Surface&)`: holds a clip rect stack and an integer translation stack
  (`save()` / `restore()`, `clipTo(Rect)`, `translate(dx, dy)`). All drawing is
  clipped to the current clip and to the surface.
- Primitives, all antialiased by per-pixel signed distance -> coverage
  (`clamp(0.5 - d, 0, 1)`), evaluated only over the shape's bounding box, with
  a fast solid-span path for pixels well inside the shape:
  - `fillRect` (integer rect, no AA, fast path for backgrounds)
  - `fillRoundedRect(RectF, radius, Color)` and a vertical two-stop gradient form
  - `strokeRoundedRect(RectF, radius, width, Color)`
  - `fillCircle(cx, cy, r, Color)`, `strokeCircle`
  - `strokeArc(cx, cy, r, startRad, endRad, width, Color)` with round caps
    (the knob's track and value arc)
  - `line(p0, p1, width, Color)` as a round-capped capsule (knob pointer)
  Coordinates are float so knob geometry is smooth; rects snap where the caller
  passes integers.
- `drawText(Font&, x, baselineY, utf8, Color)` and `textWidth`, plus helpers to
  draw text aligned left/centre/right in a rect, vertically centred on cap height.

### Layer 2: Fonts

- `Font` is created from TTF bytes in memory and a pixel size. It does not copy
  the bytes; the caller keeps them alive (embedded arrays are the normal case).
- Glyphs are rasterised with `stb_truetype` into 8-bit coverage bitmaps and kept
  in a per-`Font` cache keyed by codepoint (and, if cheap, a horizontal subpixel
  phase of 1/4 px for better spacing; builder may decide). Kerning from the font
  is applied. UTF-8 decoding is built in. No shaping, no RTL.
- A separate optional target `to_pixels::default_font` embeds JetBrains Mono
  Regular and Bold (OFL, already shipped by sndstation in `sndstation/fonts/`)
  as byte arrays via a small CMake embed script. The core target embeds no font.

### Layer 3: Components

- `Component`: `Rect bounds` (in parent coordinates), non-owning child list
  (`addChild(Component*)`, JUCE style: the app owns its components, typically as
  members), `visible`, `focusable`, a parent pointer, virtual `paint(Canvas&)`,
  virtual `bool onInput(const InputEvent&)` returning whether it consumed it, and
  `repaint()` which marks its bounds dirty on the root. Painting is clipped to the
  component's bounds; children paint after their parent.
- `Root`: the top component. Owns:
  - the `Theme` (below) and a pointer to the fonts it uses,
  - focus state and the focus navigator,
  - the dirty region,
  - key repeat state.
  API:
  - `setSize(w, h)`
  - `bool needsFrame() const` and an `std::function<void()> onNeedsFrame`
    callback fired when the first dirty rect appears after a render, so a plugin
    calls `request_frame()` from there.
  - `bool input(const InputEvent&)`
  - `void tick(double dtSeconds)` drives key repeat; returns whether anything
    changed.
  - `bool render(Surface&, uint32_t bufferAge)`: redraws what is needed and
    returns whether anything was drawn. `bufferAge == 0` redraws everything.
    `bufferAge == n` redraws the union of the damage from the last `n` renders
    plus the current dirty region; the root keeps a short history (e.g. 4
    frames) and falls back to a full redraw when `n` is older than that.
  - Dirty region: a small list of rects, merged when they overlap or the list
    grows past a few entries (then collapse to the bounding box). Repaint walks
    components whose bounds intersect each dirty rect, with the canvas clipped to
    that rect.

### Input and focus

The library defines its own input vocabulary so it does not depend on
sndstation's `pad` or the clap extension's event structs; the host glue
translates.

- `InputEvent { Input key; bool pressed; }` with
  `Input`: `Up, Down, Left, Right, Edit, Back, PagePrev, PageNext`, plus
  `EncoderDelta` carrying an `int delta` for a rotary encoder.
- The interaction model copies Bacon Beats (`bacon_beats/src/screens/app.cpp`
  `App::input`, `moveFocus`, `nudge`), which is the established feel on these
  devices:
  - d-pad alone moves focus spatially to the nearest focusable component in that
    direction, favouring ones in line (port `moveFocus`'s scoring).
  - holding `Edit` (the A / East button) makes the d-pad adjust the focused
    control: Left/Right by a fine step, Up/Down by a coarse step, with key
    repeat while held (repeat delay and rate in the theme or root config).
  - an encoder delta adjusts the focused control by fine steps without `Edit`.
  - `PagePrev` / `PageNext` are passed to the app (for tabs / pages); the root
    does not interpret them.
- Events go to the focused component first; unconsumed events bubble to parents,
  then the root applies focus navigation. So a list can consume Up/Down itself.
- The focused component draws a focus ring (theme accent, antialiased rounded
  rect). Focus changes repaint the old and new component only.
- `Root::setFocus(Component*)` lets the app place focus.

### Widgets

All take a `Theme&` from the root at paint time, carry a `std::string` label, and
call an `std::function` when the user changes them. Values set from the app
(`setValue` from a parameter change) repaint but do not fire the callback.

- **Label**: text, font choice (normal / bold / small), alignment, colour role.
  Not focusable.
- **Knob**: normalised value 0..1, optional bipolar mode (arc drawn from centre),
  default value, fine and coarse step, `std::function<std::string(float)>`
  formatter for the value text. Draws: a dark disc, a 270 degree track arc, a
  value arc in the accent colour, a pointer line, the label beneath and the
  formatted value (shown always, or only while focused; builder picks, defaulting
  to always). `onChange(float)`.
- **Slider**: horizontal or vertical, same value model and callbacks as Knob.
  Draws a rounded track, filled portion in accent, a rounded thumb, label and
  value text.
- **PresetList**: a vector of strings (preset names), a selected index, a scroll
  offset, row height from the font. While focused, Up/Down move the selection
  (and consume the event, so focus only leaves sideways or at the ends),
  `Edit` press fires `onChoose(index)`, `PagePrev/PageNext` are not consumed.
  Draws rows with the selection highlighted, keeps the selection in view, and a
  thin scroll indicator when the list is longer than the box. Optionally shows a
  category header row (string marked as header, not selectable); builder may
  defer this.

### Theme

A plain struct of colours and metrics, with names mirroring sndstation's style
(`bg0, bg1, bg2, line, txt0, txt1, txt2, accL, accR, warn`) so an app can copy
sndstation's configured colours straight in, plus corner radius, focus ring
width, and fonts (normal, bold, small, value). Defaults are a dark theme in the
spirit of sndstation's.

### The OB-Xf case (demo and acceptance)

`examples/obxf_page`: one 640x480 screen of an OB-Xf-like UI, with no dependency
on OB-Xf itself. A small fake parameter set (e.g. cutoff, resonance, env amount,
osc mix, attack, decay, sustain, release, volume), a preset list on one side,
knobs in a grid, and a slider or two. Wiring a param to a widget should look the
way it would in a real OB-Xf small-devices UI: set the value from the patch, get
`onChange` back.

Two ways to see it:
1. A headless driver that scripts a sequence of inputs and writes PNGs after each
   step (uses `stb_image_write`, example-only).
2. Optional live preview, behind `TO_PIXELS_BUILD_PREVIEW` (off by default), that
   blits the buffer to a window using a vendored single-header window library
   (fenster or minifb, builder's choice, example-only) with keyboard mapped like
   sndstation's desktop keys (arrows = d-pad, z = Edit, x = Back, q/e = page).
   This is the only place a platform dependency appears, and never in the
   library target.

A CLAP plugin implementing `pixels/1` is not part of this round; the root's API
is shaped so that glue is a few lines (`onNeedsFrame` -> `request_frame`,
`display_render` -> `render(surface, buffer_age)`).

### Testing

Catch2 tests, in a target excluded from `all`:
- Canvas: primitive coverage at known pixels (inside, edge, outside), clipping,
  stride handling with a padded buffer, no writes outside the clip.
- Fonts: measuring and drawing ASCII with the default font, cache reuse.
- Root: dirty region merging, `bufferAge` handling (0 => full, n => union of
  history), `onNeedsFrame` fires once per dirty batch, untouched pixels outside
  the dirty region stay byte-identical across renders.
- Focus: spatial navigation on a grid, bubbling, list consuming Up/Down, edit +
  d-pad steps, key repeat via `tick`.
- Golden images: render the OB-Xf page to PNGs for visual review. Exact-pixel
  goldens are optional; if used, store them and compare with a small tolerance.

## Left to the builder

- Namespace name (suggested `tpx`) and file layout under `include/to_pixels/`
  and `src/`.
- Exact SDF formulas, span fast paths, and whether to do 1/4 px glyph subpixel
  positioning.
- Dirty-rect merge heuristics and the damage history depth.
- Default theme colours and metrics, within "dark, sndstation-like".
- Key repeat timings (Bacon Beats' `repeat.h` is a reasonable source).
- Whether PresetList category headers make this round.
- Which single-header window library the preview uses.
