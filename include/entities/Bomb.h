#ifndef BOMB_H
#define BOMB_H
#include "entities/Entity.h"
#include "core/Animation.h"

class Player;

class Bomb : public Entity {
public:
    float      tiempoRestante;
    int        poderFuego;
    Player*    propietario;
    bool       recienColocada = true;

    int rRadius = 0, lRadius = 0, uRadius = 0, dRadius = 0;

    Animation* idle   = nullptr;
    Animation* expC   = nullptr;
    Animation* expUD  = nullptr;
    Animation* expLR  = nullptr;
    Animation* expR   = nullptr;
    Animation* expL   = nullptr;
    Animation* expU   = nullptr;
    Animation* expD   = nullptr;
    bool       isExploding = false;

    Bomb(float x, float y, int poder, Player* prop);
    ~Bomb();

    void initAnimations(Texture2D tiles, int playerNumber);
    void explode();
    void update(float dt) override;
    void draw()           override;
};

#endif // BOMB_H