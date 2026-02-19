#ifndef POWERUP_H
#define POWERUP_H

#include "Entity.h"

class PowerUp : public Entity {
public:
    TipoPowerUp tipoPoder;

    PowerUp(float x, float y, TipoPowerUp poder);
    void update(float dt) override;

    void draw() override;
};

#endif // POWERUP_H