# AED-Bomberman

Proyecto de Bomberman en C++ usando CMake.

## Estructura del proyecto

- `src/` : código fuente
- `include/` : headers
- `assets/` : recursos (imágenes, sonidos, etc.)
- `CMakeLists.txt` : configuración de build con CMake

## Requisitos

- Compilador con soporte C++17 o superior (g++, clang, MSVC)
- CMake 3.16+ (recomendado)
- (Si usas librerías externas) revisa `CMakeLists.txt` para dependencias

## Build (Linux/macOS)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
