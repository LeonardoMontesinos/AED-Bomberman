#include "entities/Bomb.h"

Bomb::Bomb(float x, float y, int poder, Player* prop) : Entity(x, y, 18.0f, BLACK, true, TIPO_BOMBA) {
    tiempoRestante = 3.0f;
    poderFuego = poder;
    propietario = prop;
    recienColocada = true; // Inicia permitiendo que el jugador salga
}

void Bomb::update(float dt) {
    if (activo) {
        tiempoRestante -= dt;
        if (tiempoRestante <= 0.0f) {
            activo = false;
            if (propietario) propietario->bombasActivas--;
        }

        // NUEVO: Si el jugador ya logró salir del área de la bomba, se vuelve sólida definitivamente
        if (recienColocada && propietario && !caja.intersecta(propietario->caja)) {
            recienColocada = false;
        }
    }
}