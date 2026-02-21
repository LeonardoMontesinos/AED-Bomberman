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
#include <vector>
#include <queue>
#include <cmath>

enum EstadoJuego { MENU_PRINCIPAL, PVE, PVP, PANTALLA_INFO };

class Game {
private:
    int width, height;
    int opcionMenu;
    bool debugMode;
    EstadoJuego estadoActual;
    Sound fxExplosion;
    Sound fxVictoria;
    Sound fxPickUp;
    int ganador;
    Player* jugador;
    Player* jugador2;
    Player* bot;
    Map* mapa;
    Quadtree<Entity>* quadtree;
    ListaEnlazada<Bomb*> bombas;
    ListaEnlazada<Explosion*> explosiones;
    ListaEnlazada<PowerUp*> powerups;
    bool gameOver;
    void pensarBot(float dt, float& dx, float& dy, bool& ponerBomba);
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
    void aplicarPowerUp(Player* jug, Entity* ent);
    void moverYColisionar(Player* p, float dx, float dy);
};

#endif // GAME_H