cmake_minimum_required(VERSION 3.20)
project(Game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RENDERER "GL1.1")
set(SUPPORTED_RENDERERS GDI D2D "GL" "GL1.1" "GL3.3")

list(FIND SUPPORTED_RENDERERS "${RENDERER}" _renderer_index)
if (_renderer_index EQUAL -1)
    message(FATAL_ERROR "Invalid RENDERER: ${RENDERER}")
endif()

file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/core/*.cpp
    src/game/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/platform/win32_window.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32)
    
    if (RENDERER STREQUAL "D2D")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/d2d_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE d2d1 dwrite)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DIRECT2D)
    elseif (RENDERER STREQUAL "GDI")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gdi_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GDI)
    elseif (RENDERER STREQUAL "GL")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gl_renderer.cpp src/renderer/vendor/glad.c)
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 opengl32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_33)
    elseif (RENDERER STREQUAL "GL1.1")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gl_renderer.cpp)
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 opengl32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_11)
    elseif (RENDERER STREQUAL "GL3.3")
        target_sources(${PROJECT_NAME} PRIVATE src/renderer/gl_renderer.cpp src/renderer/vendor/glad.c)
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 opengl32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_33)
    endif()
endif()