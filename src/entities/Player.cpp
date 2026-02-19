#include "entities/Player.h"

Player::Player(float x, float y) : Entity(x, y, 14.0f, BLUE, true, TIPO_JUGADOR) {
    velocidad = 200.0f;
    maxBombas = 1;
    bombasActivas = 0;
    poderFuego = 2;
    mirando = ABAJO;
}

void Player::update(float dt) {
    if (!activo) {
        color = BLACK; //efecto muerte
    }
}