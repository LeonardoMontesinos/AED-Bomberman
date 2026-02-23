#ifndef SPRITESHEETDEFS_H
#define SPRITESHEETDEFS_H

#include "Animation.h"

inline Rectangle WallDestructibleSrc()   { return {153.0f,  0.0f, 16.0f, 16.0f}; }
inline Rectangle WallIndestructibleSrc() { return {170.0f,  0.0f, 16.0f, 16.0f}; }
inline Rectangle FloorSrc(bool topRow)   { return topRow ? Rectangle{221.0f, 0.0f, 16.0f, 16.0f}
                                                          : Rectangle{204.0f, 0.0f, 16.0f, 16.0f}; }

inline Animation* NewWallDestroyAnim(Texture2D tiles) {
    return NewAnimation(
        NewSpriteSheet(tiles, 6, 6, 1, 16, 16, 1, 0,
                       {153.0f, 17.0f, 101.0f, 16.0f},
                       false, false),
        80.0f, false
    );
}

inline Animation* NewBombIdleAnim(Texture2D tiles, int playerNumber) {
    float offset = (playerNumber == 2) ? 128.0f : 0.0f;
    return NewAnimation(
        NewSpriteSheet(tiles, 4, 4, 1, 16, 16, 0, 0,
                       {offset, 173.0f, 64.0f, 16.0f},
                       false, false),
        200.0f, true
    );
}

inline Animation* NewExplosionAnim(Texture2D tiles, float xPart, int playerNumber = 1) {
    float offset = (playerNumber == 2) ? 128.0f : 0.0f;
    return NewAnimation(
        NewSpriteSheet(tiles, 5, 1, 5, 16, 16, 0, 0,
                       {xPart + offset, 189.0f, 16.0f, 80.0f},
                       true, true),
        80.0f, false
    );
}
inline Animation* NewExplosionU  (Texture2D t, int p=1){ return NewExplosionAnim(t,  0.0f, p); }
inline Animation* NewExplosionD  (Texture2D t, int p=1){ return NewExplosionAnim(t, 16.0f, p); }
inline Animation* NewExplosionL  (Texture2D t, int p=1){ return NewExplosionAnim(t, 32.0f, p); }
inline Animation* NewExplosionR  (Texture2D t, int p=1){ return NewExplosionAnim(t, 48.0f, p); }
inline Animation* NewExplosionUD (Texture2D t, int p=1){ return NewExplosionAnim(t, 64.0f, p); }
inline Animation* NewExplosionLR (Texture2D t, int p=1){ return NewExplosionAnim(t, 80.0f, p); }
inline Animation* NewExplosionC  (Texture2D t, int p=1){ return NewExplosionAnim(t, 96.0f, p); }


inline Rectangle PowerUpSrc(int tipo) {

    switch(tipo) {
        case 0: return {372.0f,  0.0f, 16.0f, 16.0f}; // UPGRADE_ADD_BOMB
        case 1: return {388.0f,  0.0f, 16.0f, 16.0f}; // UPGRADE_ADD_FIRE
        case 2: return {404.0f,  0.0f, 16.0f, 16.0f}; // UPGRADE_FULL_BOMB
        case 3: return {420.0f,  0.0f, 16.0f, 16.0f}; // UPGRADE_FULL_FIRE
        case 4: return {436.0f,  0.0f, 16.0f, 16.0f}; // UPGRADE_PASS_BOMB
        case 5: return {372.0f, 16.0f, 16.0f, 16.0f}; // UPGRADE_PASS_WALL
        case 6: return {388.0f, 16.0f, 16.0f, 16.0f}; // UPGRADE_ADD_SPEED
        case 7: return {404.0f, 16.0f, 16.0f, 16.0f}; // UPGRADE_KICK_BOMB
        case 8: return {372.0f, 32.0f, 16.0f, 16.0f}; // UPGRADE_ADD_HEALTH
        default:return {388.0f,  0.0f, 16.0f, 16.0f};
    }
}

inline Animation* NewUpgradePickupAnim(Texture2D tiles) {
    return NewAnimation(
        NewSpriteSheet(tiles, 5, 1, 5, 16, 16, 0, 1,
                       {272.0f, 51.0f, 16.0f, 85.0f},
                       true, false),
        100.0f, false
    );
}
#endif