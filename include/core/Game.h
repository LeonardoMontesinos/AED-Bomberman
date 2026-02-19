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

    // Listas independientes para manejar el ciclo de vida de los objetos
    ListaEnlazada<Bomb*> bombas;
    ListaEnlazada<Explosion*> explosiones;
    ListaEnlazada<PowerUp*> powerups;

    bool gameOver; // Estado del juego para saber si perdimos

public:
    Game(int screenWidth, int screenHeight);
    ~Game();
    void run();

private:
    void handleInput(float dt);
    void update(float dt);
    void render();

    // Función que instanciará objetos Explosion en forma de cruz al estallar una bomba
    void generarExplosion(float centroX, float centroY, int poder);

    // Función vital: Recorre todas las listas y hace 'delete' de los objetos inactivos
    void limpiarEntidadesInactivas();
};

#endif // GAME_H