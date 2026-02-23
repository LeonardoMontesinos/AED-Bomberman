    #ifndef PLAYERANIMATIONS_H
    #define PLAYERANIMATIONS_H

#include "Animation.h"

struct PlayerAnimations {
    Animation* WalkingLeft  = nullptr;
    Animation* WalkingRight = nullptr;
    Animation* WalkingUp    = nullptr;
    Animation* WalkingDown  = nullptr;
    Animation* dying        = nullptr;
    Animation* crying1      = nullptr;
    Animation* crying2      = nullptr;

    void load(Texture2D texAnims, int playerNumber) {
        float offset = (playerNumber == 2) ? 103.0f : 0.0f;


        WalkingLeft = NewAnimation(
            NewSpriteSheet(texAnims, 3, 3, 1, 16, 25, 0, 0,
                           {offset + 0.0f, 0.0f, 48.0f, 25.0f},
                           false, false),
            200.0f, true
        );
        WalkingRight = NewAnimation(
            NewSpriteSheet(texAnims, 3, 3, 1, 16, 25, 0, 0,
                           {offset + 0.0f, 26.0f, 48.0f, 25.0f},
                           false, false),
            200.0f, true
        );
        WalkingUp = NewAnimation(
            NewSpriteSheet(texAnims, 3, 3, 1, 16, 25, 0, 0,
                           {offset + 48.0f, 26.0f, 48.0f, 25.0f},
                           false, false),
            200.0f, true
        );
        WalkingDown = NewAnimation(
            NewSpriteSheet(texAnims, 3, 3, 1, 16, 25, 0, 0,
                           {offset + 48.0f, 0.0f, 48.0f, 25.0f},
                           false, false),
            200.0f, true
        );

        dying = NewAnimation(
            NewSpriteSheet(texAnims, 4, 4, 1, 16, 25, 0, 0,
                           {offset + 0.0f, 51.0f, 63.0f, 26.0f},
                           false, false),
            60.0f, true
        );

        crying1 = NewAnimation(
            NewSpriteSheet(texAnims, 3, 3, 1, 16, 24, 0, 0,
                           {offset + 0.0f, 78.0f, 48.0f, 24.0f},
                           false, false),
            200.0f, false
        );
        crying2 = NewAnimation(
            NewSpriteSheet(texAnims, 2, 2, 1, 16, 24, 0, 0,
                           {offset + 48.0f, 78.0f, 31.0f, 24.0f},
                           false, false),
            200.0f, true
        );
    }

    ~PlayerAnimations() {
        delete WalkingLeft;  delete WalkingRight;
        delete WalkingUp;    delete WalkingDown;
        delete dying;        delete crying1; delete crying2;
    }
};

#endif
