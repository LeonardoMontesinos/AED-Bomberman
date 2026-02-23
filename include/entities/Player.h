#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

enum Direccion { ARRIBA, ABAJO, IZQUIERDA, DERECHA };

class Player : public Entity {
public:
    float velocidad;
    int maxBombas;
    int bombasActivas;
    int poderFuego;
    Direccion mirando;
    float timerFuego = 0.0f;
    float timerVelocidad = 0.0f;
    Player(float x, float y);
    void update(float dt) override;
};

#endif // PLAYER_H