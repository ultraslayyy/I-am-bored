cmake_minimum_required(VERSION 3.20)
project(Game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckIncludeFile)
include(CheckLibraryExists)

check_include_file(X11/Xlib.h HAS_X11)
Check_library_exists(X11 XOpenDisplay "" HAS_XOPEN_DISPLAY)

set(RENDERER "D3D11" CACHE STRING "Renderer")
set_property(CACHE RENDERER PROPERTY STRINGS GDI D2D D3D11 GL "GL1.1" "GL3.3")

set(OPT_PROFILE "SIZE" CACHE STRING "Option Profile")
set_property(CACHE OPT_PROFILE PROPERTY STRINGS PERF SIZE BALANCED DEV)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/main.cpp
    src/core/*.cpp
    src/game/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if (HAS_X11 AND HAS_XOPEN_DISPLAY)
    add_compile_options(${PROJECT_NAME} PRIVATE HAS_X11)
endif()

if (NOT OPT_PROFILE STREQUAL "DEV")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

    if (ipo_supported)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO/LTO not supported ${ipo_error}")
    endif()
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
    # target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)
endif()

if (MSVC)
    if (OPT_PROFILE STREQUAL "DEV")
        target_compile_options(${PROJECT_NAME} PRIVATE /Od /Zi /MP)
        target_link_options(${PROJECT_NAME} PRIVATE /DEBUG)
    elseif (OPT_PROFILE STREQUAL "PERF")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /GL /Ot)
    elseif (OPT_PROFILE STREQUAL "SIZE")
        target_compile_options(${PROJECT_NAME} PRIVATE /O1 /GL /GS- /Zl)
        target_link_options(${PROJECT_NAME} PRIVATE /LTCG /OPT:REF /OPT:ICF /ENTRY:mainCRTStartup /INCREMENTAL:NO)
    elseif (OPT_PROFILE STREQUAL "BALANCED")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /GL /Gy)
        target_link_options(${PROJECT_NAME} PRIVATE /LTCG /OPT:REF /OPT:ICF)
    endif()

    target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
else()
    if (OPT_PROFILE STREQUAL "DEV")
        target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
    elseif (OPT_PROFILE STREQUAL "PERF")
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native)
    elseif (OPT_PROFILE STREQUAL "SIZE")
        target_compile_options(${PROJECT_NAME} PRIVATE -Os -ffunction-sections -fdata-sections)
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections -Wl,-s -Wl,--strip-all)
    elseif (OPT_PROFILE STREQUAL "BALANCED")
        target_compile_options(${PROJECT_NAME} PRIVATE -O2 -ffunction-sections -fdata-sections)
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
    endif()

    if (WIN32)
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE OPT_PROFILE=${OPT_PROFILE})

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/platform/win32_window.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32)

    if (RENDERER MATCHES "^GL")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gl_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 opengl32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL)

        if (RENDERER STREQUAL "GL" OR RENDERER STREQUAL "GL3.3")
            target_sources(${PROJECT_NAME} PRIVATE src/renderer/vendor/glad.c)
            target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_33)
            file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/fonts" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/fonts")
        elseif (RENDERER STREQUAL "GL1.1")
            target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_11)
        endif()
    elseif (RENDERER STREQUAL "D2D")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/d2d_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE d2d1 dwrite)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DIRECT2D)
    elseif (RENDERER STREQUAL "D3D11")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/d3d11_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi d3dcompiler d2d1 dwrite)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DIRECT3D11)
    elseif (RENDERER STREQUAL "GDI")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gdi_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GDI)
    endif()
else()
    # Linux / X11 Support
    target_sources(${PROJECT_NAME} PRIVATE src/platform/x11_window.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11 GL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_X11)

    if (RENDERER MATCHES "^GL")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gl_renderer.cpp)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL)

        if (RENDERER STREQUAL "GL" OR RENDERER STREQUAL "GL3.3")
            target_sources(${PROJECT_NAME} PRIVATE src/renderer/vendor/glad.c)
            target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_33)
        elseif (RENDERER STREQUAL "GL1.1")
            target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_11)
        endif()
    endif()

endif()