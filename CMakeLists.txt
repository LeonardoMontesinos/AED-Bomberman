cmake_minimum_required(VERSION 4.1)
project(Bomberman)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/*.h")

include_directories(include)

add_executable(Bomberman ${SRC_FILES})

find_package(raylib REQUIRED)


target_link_libraries(Bomberman PRIVATE raylib)