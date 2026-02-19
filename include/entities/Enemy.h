#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Enemy : public Entity {
public:
    float velocidad;
    Enemy(float x, float y);
    void update(float dt) override;
};



#endif // ENEMY_H