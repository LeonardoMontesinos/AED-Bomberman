cmake_minimum_required(VERSION 3.20)
project(Bomberman)
set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/*.h")
add_executable(Bomberman ${SRC_FILES})
target_include_directories(Bomberman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(FetchContent)
FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)
target_link_libraries(Bomberman PRIVATE raylib)