#include "core/Map.h"
#include <stdlib.h>

static const float TILE_SIZE = 40.0f;
static const float MEDIO     = TILE_SIZE / 2.0f;
static const int   COLUMNAS  = 920 / (int)TILE_SIZE;   // 20
static const int   FILAS     = 680 / (int)TILE_SIZE;   // 15

Map::Map() {}

Map::~Map() {
    auto actual = muros.cabeza;
    while (actual) {
        delete actual->dato;
        actual = actual->siguiente;
    }
}

void Map::insertarMuro(float x, float y, float medio, bool destructible) {
    muros.insertar(new WallEntity(x, y, medio, destructible, texTiles));
}

void Map::cargarMapa(int tipo) {
    auto actual = muros.cabeza;
    while (actual) { delete actual->dato; actual = actual->siguiente; }
    muros.limpiar();

    if (tipo == 1) generarMapaClasico();
    else if (tipo == 2) generarMapaArena();
    else generarMapaClasico();
}

void Map::generarMapaClasico() {
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            float x = col * TILE_SIZE + MEDIO;
            float y = fila * TILE_SIZE + MEDIO;

            if (fila == 0 || fila == FILAS-1 || col == 0 || col == COLUMNAS-1) {
                insertarMuro(x, y, MEDIO, false);
                continue;
            }

            bool zonaJ1 = (fila==1 && col==1) || (fila==1 && col==2) || (fila==2 && col==1);
            bool zonaJ2 = (fila==FILAS-2 && col==COLUMNAS-2) ||
                          (fila==FILAS-2 && col==COLUMNAS-3) ||
                          (fila==FILAS-3 && col==COLUMNAS-2);
            if (zonaJ1 || zonaJ2) continue;

            if (fila % 2 == 0 && col % 2 == 0) {
                insertarMuro(x, y, MEDIO, false);
                continue;
            }

            if (rand() % 100 < 75) {
                insertarMuro(x, y, MEDIO, true);
            }
        }
    }
}

void Map::generarMapaArena() {
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            float x = col * TILE_SIZE + MEDIO;
            float y = fila * TILE_SIZE + MEDIO;

            if (fila == 0 || fila == FILAS-1 || col == 0 || col == COLUMNAS-1) {
                insertarMuro(x, y, MEDIO, false);
                continue;
            }

            bool esCentro = (fila >= 4 && fila <= FILAS-5) &&
                            (col >= 5 && col <= COLUMNAS-6);

            if (esCentro) {
                if (rand() % 100 < 30)
                    insertarMuro(x, y, MEDIO, true);
            } else {
                if (fila % 3 == 0 && col % 3 == 0) {
                    insertarMuro(x, y, MEDIO, false);
                } else {
                    bool esZonaSegura = (fila <= 2 && col <= 2);
                    if (!esZonaSegura && rand() % 100 < 85)
                        insertarMuro(x, y, MEDIO, true);
                }
            }
        }
    }
}

void Map::update(float dt) {
    auto actual = muros.cabeza;
    while (actual) {
        WallEntity* w = static_cast<WallEntity*>(actual->dato);
        if (w->destruyendo) {
            bool terminado = w->updateDestroy(dt);
            if (terminado) w->activo = false;
        }
        actual = actual->siguiente;
    }
}

void Map::draw() {
    auto actual = muros.cabeza;
    while (actual) {
        if (actual->dato->activo) {
            WallEntity* w = static_cast<WallEntity*>(actual->dato);
            w->drawSprite(texTiles);
        }
        actual = actual->siguiente;
    }
}