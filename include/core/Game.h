#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include "core/Map.h"
#include "entities/Player.h"
#include "entities/Bomb.h"
#include "entities/Explosion.h"
#include "entities/PowerUp.h"
#include "utils/Quadtree.h"
#include "utils/ListaEnlazada.h"

class Game {
private:
    int width, height;
    Player* jugador;
    Map* mapa;
    Quadtree<Entity>* quadtree;

    ListaEnlazada<Bomb*> bombas;
    ListaEnlazada<Explosion*> explosiones;
    ListaEnlazada<PowerUp*> powerups;

    bool gameOver;
    Camera2D camara;

    int totalEntidades;
    int colisionesComprobadas;
    int entidadesCercanas;

public:
    Game(int screenWidth, int screenHeight);
    ~Game();
    void run();

private:
    void handleInput(float dt);
    void update(float dt);
    void render();
    void generarExplosion(float centroX, float centroY, int poder);
    void limpiarEntidadesInactivas();
};

#endif // GAME_H