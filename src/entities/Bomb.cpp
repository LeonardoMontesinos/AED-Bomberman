#include "entities/Bomb.h"

Bomb::Bomb(float x, float y, int poder, Player* prop) : Entity(x, y, 18.0f, BLACK, true, TIPO_BOMBA) {
    tiempoRestante = 3.0f;
    poderFuego = poder;
    propietario = prop;
    recienColocada = true;
}

void Bomb::update(float dt) {
    if (activo) {
        tiempoRestante -= dt;
        if (tiempoRestante <= 0.0f) {
            activo = false;
            if (propietario) propietario->bombasActivas--;
        }

        if (recienColocada && propietario && !caja.intersecta(propietario->caja)) {
            recienColocada = false;
        }
    }
}