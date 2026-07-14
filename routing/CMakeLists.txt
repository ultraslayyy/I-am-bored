cmake_minimum_required(VERSION 3.20)
project(Routing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/main.cpp
    src/app/app.cpp
    src/app/cli.cpp
    src/geo/geo.cpp
    src/graph/graphBuilder.cpp
    src/navigation/Directions.cpp
    src/osm/OSMParser.cpp
    src/routing/AStarRouter.cpp
    vendor/tinyxml2/tinyxml2.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})