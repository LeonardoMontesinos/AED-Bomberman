#ifndef MAP_H
#define MAP_H

#include "utils/ListaEnlazada.h"
#include "entities/Entity.h"
#include "core/Animation.h"
#include "core/SpriteSheetDefs.h"

struct WallEntity : public Entity {
    bool        destructible;
    bool        destruyendo = false;
    Animation*  destroyAnim = nullptr;
    WallEntity(float x, float y, float medio, bool dest, Texture2D texTiles)
        : Entity(x, y, medio, dest ? BROWN : DARKGRAY, true,
                 dest ? TIPO_MURO_DESTRUCTIBLE : TIPO_MURO_INDESTRUCTIBLE),
          destructible(dest)
    {
        if (dest) {
            destroyAnim = NewWallDestroyAnim(texTiles);
        }
    }

    ~WallEntity() { delete destroyAnim; }

    void startDestroy() {
        if (!destructible || destruyendo) return;
        destruyendo = true;
        destroyAnim->Play();
    }

    bool updateDestroy(float dt) {
        if (!destruyendo) return false;
        destroyAnim->Update(dt);
        return !destroyAnim->IsPlaying();
    }

    void drawSprite(Texture2D texTiles) const {
        Rectangle dst = {
            caja.centro.x - 20.0f,
            caja.centro.y - 20.0f,
            40.0f, 40.0f
        };
        if (destruyendo && destroyAnim) {
            destroyAnim->DrawAt(dst);
        } else {
            Rectangle src = destructible
                ? WallDestructibleSrc()    // x=153, y=0
                : WallIndestructibleSrc(); // x=170, y=0
            DrawTexturePro(texTiles, src, dst, {0,0}, 0.0f, WHITE);
        }
    }
};

class Map {
public:
    ListaEnlazada<Entity*> muros;
    Texture2D texTiles;

    Map();
    ~Map();

    void setTileTexture(Texture2D t) { texTiles = t; }
    void cargarMapa(int tipo);
    void update(float dt);
    void draw();

private:
    void generarMapaClasico();
    void generarMapaArena();
    void insertarMuro(float x, float y, float medio, bool destructible);
};

#endif // MAP_H