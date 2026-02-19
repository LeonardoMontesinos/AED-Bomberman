#include "core/Map.h"
#include <stdlib.h>

Map::Map() {}

Map::~Map() {
    auto actual = muros.cabeza;
    while (actual != nullptr) {
        delete actual->dato;
        actual = actual->siguiente;
    }
}

void Map::cargarMapa(int tipo) {
    auto actual = muros.cabeza;
    while (actual != nullptr) {
        delete actual->dato;
        actual = actual->siguiente;
    }
    muros.limpiar();

    if (tipo == 1) generarMapaClasico();
    else if (tipo == 2) generarMapaArena();
    else generarMapaClasico();
}

void Map::generarMapaClasico() {
    float TILE_SIZE = 40.0f;
    float MEDIO = TILE_SIZE / 2.0f;

    int COLUMNAS = 40;
    int FILAS = 40;

    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            float x = col * TILE_SIZE + MEDIO;
            float y = fila * TILE_SIZE + MEDIO;

            if (fila == 0 || fila == FILAS - 1 || col == 0 || col == COLUMNAS - 1) {
                muros.insertar(new Entity(x, y, MEDIO, DARKGRAY, true, TIPO_MURO_INDESTRUCTIBLE));
            }
            else if (fila % 2 == 0 && col % 2 == 0) {
                muros.insertar(new Entity(x, y, MEDIO, DARKGRAY, true, TIPO_MURO_INDESTRUCTIBLE));
            }
            else {
                bool esZonaSegura = (fila <= 2 && col <= 2);
                if (!esZonaSegura && rand() % 100 < 70) {
                    muros.insertar(new Entity(x, y, MEDIO, BROWN, true, TIPO_MURO_DESTRUCTIBLE));
                }
            }
        }
    }
}

void Map::generarMapaArena() {
    float TILE_SIZE = 40.0f;
    float MEDIO = TILE_SIZE / 2.0f;
    int COLUMNAS = 800 / (int)TILE_SIZE;
    int FILAS = 600 / (int)TILE_SIZE;

    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            float x = col * TILE_SIZE + MEDIO;
            float y = fila * TILE_SIZE + MEDIO;

            if (fila == 0 || fila == FILAS - 1 || col == 0 || col == COLUMNAS - 1) {
                muros.insertar(new Entity(x, y, MEDIO, DARKGRAY, true, TIPO_MURO_INDESTRUCTIBLE));
                continue;
            }

            bool esCentro = (fila >= 4 && fila <= FILAS - 5) && (col >= 5 && col <= COLUMNAS - 6);

            if (esCentro) {
                if (rand() % 100 < 30) {
                    muros.insertar(new Entity(x, y, MEDIO, BROWN, true, TIPO_MURO_DESTRUCTIBLE));
                }
            }
            else {
                if (fila % 3 == 0 && col % 3 == 0) {
                    muros.insertar(new Entity(x, y, MEDIO, DARKGRAY, true, TIPO_MURO_INDESTRUCTIBLE));
                } else {
                    bool esZonaSegura = (fila <= 2 && col <= 2);
                    if (!esZonaSegura && rand() % 100 < 85) {
                        // AÑADIDO: TIPO_MURO_DESTRUCTIBLE
                        muros.insertar(new Entity(x, y, MEDIO, BROWN, true, TIPO_MURO_DESTRUCTIBLE));
                    }
                }
            }
        }
    }
}

void Map::draw() {
    auto actual = muros.cabeza;
    while (actual != nullptr) {
        actual->dato->draw();
        actual = actual->siguiente;
    }
}