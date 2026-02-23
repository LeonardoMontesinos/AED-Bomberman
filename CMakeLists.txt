cmake_minimum_required(VERSION 3.20)
project(Bomberman)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/*.h")

add_executable(Bomberman ${SRC_FILES})

target_include_directories(Bomberman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

target_link_libraries(Bomberman PRIVATE raylib)

# --- CONFIGURACIÓN EXCLUSIVA PARA WEB (EMSCRIPTEN) ---
if (EMSCRIPTEN)
    # 1. Cambiamos la extensión final del ejecutable a .html
    set_target_properties(Bomberman PROPERTIES SUFFIX ".html")

    # 2. Le decimos al compilador que empaquete la carpeta "assets" virtualmente
    #    y que use el entorno de GLFW para Raylib
    target_link_options(Bomberman PRIVATE
            "-s" "USE_GLFW=3"
            "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets@assets"
    )
endif()