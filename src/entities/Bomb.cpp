#include "entities/Bomb.h"
#include "entities/Player.h"
#include "core/SpriteSheetDefs.h"
#include <cmath>

Bomb::Bomb(float x, float y, int poder, Player* prop)
    : Entity(x, y, 16.0f, DARKGRAY, true, TIPO_BOMBA),
      tiempoRestante(3.0f),
      poderFuego(poder),
      propietario(prop),
      recienColocada(true)
{}

Bomb::~Bomb() {
    delete idle;
    delete expC;  delete expUD; delete expLR;
    delete expR;  delete expL;
    delete expU;  delete expD;
}

void Bomb::initAnimations(Texture2D tiles, int playerNumber) {
    idle  = NewBombIdleAnim(tiles, playerNumber);
    expC  = NewExplosionC (tiles, playerNumber);
    expUD = NewExplosionUD(tiles, playerNumber);
    expLR = NewExplosionLR(tiles, playerNumber);
    expR  = NewExplosionR (tiles, playerNumber);
    expL  = NewExplosionL (tiles, playerNumber);
    expU  = NewExplosionU (tiles, playerNumber);
    expD  = NewExplosionD (tiles, playerNumber);
    idle->Play();
}

void Bomb::explode() {
    isExploding = true;
    if (expC)  expC->Play();
    if (expUD) expUD->Play();
    if (expLR) expLR->Play();
    if (expR)  expR->Play();
    if (expL)  expL->Play();
    if (expU)  expU->Play();
    if (expD)  expD->Play();
}

void Bomb::update(float dt) {
    if (!activo) return;

    if (recienColocada && propietario != nullptr && propietario->activo) {
        float dx = std::abs(caja.centro.x - propietario->caja.centro.x);
        float dy = std::abs(caja.centro.y - propietario->caja.centro.y);

        if (dx > 30.0f || dy > 30.0f) {
            recienColocada = false;
        }
    } else if (propietario == nullptr || !propietario->activo) {
        recienColocada = false;
    }

    tiempoRestante -= dt;

    if (isExploding) {
        if (expC)  expC->Update(dt);
        if (expUD) expUD->Update(dt);
        if (expLR) expLR->Update(dt);
        if (expR)  expR->Update(dt);
        if (expL)  expL->Update(dt);
        if (expU)  expU->Update(dt);
        if (expD)  expD->Update(dt);

        if (expC && !expC->IsPlaying()) activo = false;
    } else {
        if (idle) idle->Update(dt);
    }
}

void Bomb::draw() {
    if (!activo) return;

    const float TILE = 40.0f;
    Rectangle center = {
        caja.centro.x - TILE / 2.0f,
        caja.centro.y - TILE / 2.0f,
        TILE, TILE
    };

    if (!isExploding) {
        if (idle) idle->DrawAt(center);
        return;
    }

    if (!expC || !expC->IsPlaying()) return;

    expC->DrawAt(center);

    for (int i = 0; i < rRadius; i++) {
        Rectangle dst = { center.x + TILE * (i + 1), center.y, TILE, TILE };
        ((i == rRadius - 1) ? expR : expLR)->DrawAt(dst);
    }
    for (int i = 0; i < lRadius; i++) {
        Rectangle dst = { center.x - TILE * (i + 1), center.y, TILE, TILE };
        ((i == lRadius - 1) ? expL : expLR)->DrawAt(dst);
    }
    for (int i = 0; i < uRadius; i++) {
        Rectangle dst = { center.x, center.y - TILE * (i + 1), TILE, TILE };
        ((i == uRadius - 1) ? expU : expUD)->DrawAt(dst);
    }
    for (int i = 0; i < dRadius; i++) {
        Rectangle dst = { center.x, center.y + TILE * (i + 1), TILE, TILE };
        ((i == dRadius - 1) ? expD : expUD)->DrawAt(dst);
    }
}