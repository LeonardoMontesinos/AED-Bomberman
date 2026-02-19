#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "Entity.h"

class Explosion : public Entity {
public:
    float tiempoRestante;

    Explosion(float x, float y);
    void update(float dt) override;
};

#endif // EXPLOSION_H