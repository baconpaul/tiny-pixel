cmake_minimum_required(VERSION 3.21)
project(tiny-pixel VERSION 0.1.0 LANGUAGES CXX)

option(TINY_PIXEL_BUILD_TESTS "Build the tiny-pixel tests" ${PROJECT_IS_TOP_LEVEL})
option(TINY_PIXEL_BUILD_EXAMPLES "Build the tiny-pixel examples" ${PROJECT_IS_TOP_LEVEL})

if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif ()

include(cmake/embed.cmake)

function(tpx_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else ()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif ()
endfunction()

add_library(tiny-pixel STATIC
    src/surface.cpp
    src/canvas.cpp
    src/canvas_shapes.cpp
    src/font.cpp
    src/damage.cpp
    src/component.cpp
    src/root.cpp
    src/focus.cpp
    src/widgets/value_control.cpp
    src/widgets/knob.cpp
    src/widgets/slider.cpp
    src/widgets/label.cpp
    src/widgets/preset_list.cpp
)
add_library(tiny-pixel::tiny-pixel ALIAS tiny-pixel)
target_include_directories(tiny-pixel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(tiny-pixel SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)
target_compile_features(tiny-pixel PUBLIC cxx_std_20)
tpx_warnings(tiny-pixel)

# optional: jetbrains mono (OFL), so an app need not ship its own font
add_library(tiny-pixel-default-font STATIC src/default_font.cpp)
add_library(tiny-pixel::default-font ALIAS tiny-pixel-default-font)
tpx_embed(tiny-pixel-default-font tpx_font_jbm_regular
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/JetBrainsMono-Regular.ttf)
tpx_embed(tiny-pixel-default-font tpx_font_jbm_bold
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/JetBrainsMono-Bold.ttf)
target_link_libraries(tiny-pixel-default-font PUBLIC tiny-pixel)

if (TINY_PIXEL_BUILD_TESTS OR TINY_PIXEL_BUILD_EXAMPLES)
    # png writing for shots, never part of tiny-pixel itself
    add_library(tpx-png STATIC examples/common/png.cpp)
    target_include_directories(tpx-png PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/examples/common)
    target_include_directories(tpx-png SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)
    target_link_libraries(tpx-png PUBLIC tiny-pixel)
endif ()

if (TINY_PIXEL_BUILD_EXAMPLES)
    add_library(tpx-obxf-page STATIC examples/obxf_page/obxf_page.cpp)
    target_include_directories(tpx-obxf-page PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/examples/obxf_page)
    target_link_libraries(tpx-obxf-page PUBLIC tiny-pixel-default-font)
    tpx_warnings(tpx-obxf-page)

    add_executable(obxf_shots examples/obxf_page/shots.cpp)
    target_link_libraries(obxf_shots PRIVATE tpx-obxf-page tpx-png)
    tpx_warnings(obxf_shots)
endif ()

if (TINY_PIXEL_BUILD_TESTS)
    add_executable(tiny-pixel-tests EXCLUDE_FROM_ALL
        tests/test_main.cpp
        tests/canvas_test.cpp
        tests/font_test.cpp
        tests/damage_test.cpp
        tests/root_test.cpp
        tests/focus_test.cpp
        tests/widgets_test.cpp
        tests/preset_list_test.cpp
        tests/shots_test.cpp
    )
    target_include_directories(tiny-pixel-tests SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/catch2)
    if (TINY_PIXEL_BUILD_EXAMPLES)
        target_sources(tiny-pixel-tests PRIVATE tests/obxf_page_test.cpp)
        target_link_libraries(tiny-pixel-tests PRIVATE tpx-obxf-page tpx-png)
    else ()
        target_link_libraries(tiny-pixel-tests PRIVATE tiny-pixel-default-font tpx-png)
    endif ()
    tpx_warnings(tiny-pixel-tests)
endif ()
