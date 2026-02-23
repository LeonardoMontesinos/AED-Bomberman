#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "core/PlayerAnimations.h"

enum Direccion { ARRIBA, ABAJO, IZQUIERDA, DERECHA };

class Player : public Entity {
public:

    float      velocidad;
    int        maxBombas;
    int        bombasActivas;
    int        poderFuego;
    float      timerFuego;
    float      timerVelocidad;
    float      scale;
    Direccion  mirando;

    // ── Animaciones ──────────────────────────────────────────
    PlayerAnimations Animations;

    bool  isAlive;
    float diedAt;

    Player(float x, float y);

    void setTexture(Texture2D texAnims, int playerNumber);
    void update(float dt) override;
    void draw() override;

    void updateAnimX(bool movingRight);
    void updateAnimY(bool movingDown);
    void stopAnimX();
    void stopAnimY();
    void die();
};

#endif // PLAYER_H