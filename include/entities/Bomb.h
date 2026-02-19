#ifndef BOMB_H
#define BOMB_H

#include "Entity.h"
#include "Player.h"

class Bomb : public Entity {
public:
    float tiempoRestante;
    int poderFuego;
    Player* propietario;
    bool recienColocada;

    Bomb(float x, float y, int poder, Player* prop);
    void update(float dt) override;
};

#endif // BOMB_H