cmake_minimum_required(VERSION 3.20)
project(Game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RENDERER "GDI")
set(SUPPORTED_RENDERERS D2D GDI)

list(FIND SUPPORTED_RENDERERS "${RENDERER}" _renderer_index)
if (_renderer_index EQUAL -1)
    message(FATAL_ERROR "Invalid RENDERER: ${RENDERER}")
endif()

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32)
    
    if (RENDERER STREQUAL "D2D")
        target_link_libraries(${PROJECT_NAME} PRIVATE d2d1 dwrite)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DIRECT2D)
    elseif (RENDERER STREQUAL "GDI")
        target_link_libraries(${PROJECT_NAME} PRIVATE gdi32)
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GDI)
    endif()
endif()