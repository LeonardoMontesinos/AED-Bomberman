#include "entities/Enemy.h"

Enemy::Enemy(float x, float y) : Entity(x, y, 20.0f, RED, true, TIPO_ENEMIGO) {
    velocidad = 100.0f;
}

void Enemy::update(float dt) {
}