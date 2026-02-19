#include "entities/Explosion.h"

Explosion::Explosion(float x, float y) : Entity(x, y, 20.0f, ORANGE, false, TIPO_EXPLOSION) {
    tiempoRestante = 0.5f;
}

void Explosion::update(float dt) {
    tiempoRestante -= dt;
    if (tiempoRestante <= 0.0f) {
        activo = false;
    }
}