#include "entities/Enemy.h"

// AÑADIDO: TIPO_ENEMIGO al final del constructor de Entity
Enemy::Enemy(float x, float y) : Entity(x, y, 20.0f, RED, true, TIPO_ENEMIGO) {
    velocidad = 100.0f;
}

void Enemy::update(float dt) {
    // La IA de movimiento la implementaremos aquí más adelante
}