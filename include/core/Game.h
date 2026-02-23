#ifndef GAME_H
#define GAME_H
// ============================================================
// Game.h — Actualizado con nuevas firmas
// ============================================================
#include "core/Map.h"
#include "core/SpriteSheetDefs.h"
#include "entities/Player.h"
#include "entities/Bomb.h"
#include "entities/Explosion.h"
#include "entities/PowerUp.h"
#include "utils/ListaEnlazada.h"
#include "utils/Quadtree.h"
#include <raylib.h>
#include <vector>
#include <queue>

enum EstadoJuego { MENU_PRINCIPAL, PVP, PVE, PANTALLA_INFO };

class Game {
public:
    Game(int screenWidth, int screenHeight);
    ~Game();
    void run();
    void update(float dt);
    void render();

private:
    int            width, height;
    EstadoJuego    estadoActual;
    int            opcionMenu;
    bool           debugMode;
    bool           gameOver;
    int            ganador;

    Player*                jugador;
    Player*                jugador2;
    Player*                bot;
    Map*                   mapa;
    ListaEnlazada<Entity*> bombas;
    ListaEnlazada<Entity*> explosiones;
    ListaEnlazada<Entity*> powerups;
    Quadtree<Entity>*      quadtree;

    Texture2D texMenu;
    Texture2D texGameOver;
    Texture2D texBomberman;
    Texture2D texTiles;
    Texture2D texArena;

    Sound fxExplosion;
    Sound fxVictoria;
    Sound fxPickUp;


    void handleInput(float dt);
    void limpiarEntidadesInactivas();
    void aplicarPowerUp(Player* jug, Entity* ent);
    void moverYColisionar(Player* p, float dx, float dy);
    void pensarBot(float dt, float& dx, float& dy, bool& ponerBomba);

    Bomb* crearBomba(float cx, float cy, int poder, Player* prop, int playerNumber);
    void  generarExplosion(Bomb* bomba);
};

#endif // GAME_H