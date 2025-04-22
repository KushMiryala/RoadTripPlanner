cmake_minimum_required(VERSION 3.25)
project(Project3_RoadTrip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_DIR "C:/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(Project3_RoadTrip
        main.cpp
        ui.cpp
        dijkstra.cpp
        geo_utils.cpp
        file_utils.cpp
        kdtree.cpp
)
target_link_libraries(Project3_RoadTrip
        sfml-graphics sfml-window sfml-system
)
