cmake_minimum_required(VERSION 3.21)
project(to_pixels VERSION 0.1.0 LANGUAGES CXX)

option(TO_PIXELS_BUILD_TESTS "Build the to_pixels tests" ${PROJECT_IS_TOP_LEVEL})
option(TO_PIXELS_BUILD_EXAMPLES "Build the to_pixels examples" ${PROJECT_IS_TOP_LEVEL})

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

add_library(to_pixels STATIC
    src/surface.cpp
    src/canvas.cpp
    src/canvas_shapes.cpp
    src/font.cpp
    src/damage.cpp
    src/component.cpp
    src/root.cpp
    src/focus.cpp
)
add_library(to_pixels::to_pixels ALIAS to_pixels)
target_include_directories(to_pixels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(to_pixels SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)
target_compile_features(to_pixels PUBLIC cxx_std_20)
tpx_warnings(to_pixels)

# optional: jetbrains mono (OFL), so an app need not ship its own font
add_library(to_pixels_default_font STATIC src/default_font.cpp)
add_library(to_pixels::default_font ALIAS to_pixels_default_font)
tpx_embed(to_pixels_default_font tpx_font_jbm_regular
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/JetBrainsMono-Regular.ttf)
tpx_embed(to_pixels_default_font tpx_font_jbm_bold
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/JetBrainsMono-Bold.ttf)
target_link_libraries(to_pixels_default_font PUBLIC to_pixels)

if (TO_PIXELS_BUILD_TESTS)
    add_executable(to_pixels_tests EXCLUDE_FROM_ALL
        tests/test_main.cpp
        tests/png.cpp
        tests/canvas_test.cpp
        tests/font_test.cpp
        tests/damage_test.cpp
        tests/root_test.cpp
        tests/focus_test.cpp
        tests/shots_test.cpp
    )
    target_include_directories(to_pixels_tests SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/catch2
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)
    target_link_libraries(to_pixels_tests PRIVATE to_pixels_default_font)
    tpx_warnings(to_pixels_tests)
endif ()
