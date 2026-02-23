#include "entities/Player.h"
#include <raylib.h>

Player::Player(float x, float y) : Entity(x, y, 12.0f, WHITE, true, TIPO_JUGADOR) {
    velocidad    = 150.0f;
    maxBombas    = 1;
    bombasActivas= 0;
    poderFuego   = 1;
    timerFuego   = 0.0f;
    timerVelocidad = 0.0f;
    scale        = 2.0f;
    mirando      = ABAJO;
    isAlive      = true;
    diedAt       = -1.0f;
}

void Player::setTexture(Texture2D texAnims, int playerNumber) {
    Animations.load(texAnims, playerNumber);

    Animations.WalkingDown->Play();
    Animations.WalkingUp->Play();
    Animations.WalkingLeft->Play();
    Animations.WalkingRight->Play();
}

void Player::update(float dt) {
    if (!activo) return;

    if (!isAlive) {
        Animations.dying->Update(dt);
        Animations.crying1->Update(dt);
        Animations.crying2->Update(dt);
        return;
    }
}

void Player::updateAnimX(bool movingRight) {
    if (movingRight) {
        mirando = DERECHA;
        Animations.WalkingRight->Update(GetFrameTime());
        Animations.WalkingLeft->Index = 0;
    } else {
        mirando = IZQUIERDA;
        Animations.WalkingRight->Index = 0;
        Animations.WalkingLeft->Update(GetFrameTime());
    }
}

void Player::updateAnimY(bool movingDown) {
    if (movingDown) {
        mirando = ABAJO;
        Animations.WalkingDown->Update(GetFrameTime());
        Animations.WalkingUp->Index = 1;
    } else {
        mirando = ARRIBA;
        Animations.WalkingDown->Index = 1;
        Animations.WalkingUp->Update(GetFrameTime());
    }
}

void Player::stopAnimX() {
    Animations.WalkingLeft->Index = 0;
    Animations.WalkingRight->Index = 0;
}

void Player::stopAnimY() {
    Animations.WalkingUp->Index = 1;
    Animations.WalkingDown->Index = 1;
}

void Player::draw() {
    if (!activo) return;


    float offsetY = 14.0f;

    if (isAlive) {
        switch (mirando) {
            case ARRIBA:
                Animations.WalkingUp->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
                break;
            case ABAJO:
                Animations.WalkingDown->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
                break;
            case IZQUIERDA:
                Animations.WalkingLeft->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
                break;
            case DERECHA:
                Animations.WalkingRight->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
                break;
        }
    } else {
        //Secuencia muerte
        float elapsed = GetTime() - diedAt;
        if (elapsed < 0.5f) {
            Animations.dying->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
        } else if (elapsed < 1.5f) {
            if (!Animations.crying1->IsPlaying()) {
                Animations.crying1->Play();
            }
            Animations.crying1->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
        } else if (elapsed < 3.2f) {
            if (!Animations.crying2->IsPlaying()) {
                Animations.crying2->Play();
            }
            Animations.crying2->DrawCenteredAtWithScale(caja.centro.x, caja.centro.y - offsetY, scale);
        } else {
            activo = false;
        }
    }
}

void Player::die() {
    if (!isAlive) return;

    isAlive = false;

    diedAt = GetTime();
    Animations.dying->Play();
}