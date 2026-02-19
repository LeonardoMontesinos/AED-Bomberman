#ifndef ENTITY_H
#define ENTITY_H

#include <raylib.h>
#include "core/Types.h"

class Entity {
public:
    AABB caja;
    Color color;
    bool solido;
    bool activo; // Si es false, se borrará en el próximo frame
    TipoEntidad tipo;

    Entity(float x, float y, float medio, Color col, bool sol, TipoEntidad t) {
        caja.centro = {x, y};
        caja.medio = medio;
        color = col;
        solido = sol;
        tipo = t;
        activo = true;
    }

    virtual ~Entity() {}
    virtual void update(float dt) {}

    virtual void draw() {
        if (activo) {
            DrawRectangle((int)(caja.centro.x - caja.medio),
                          (int)(caja.centro.y - caja.medio),
                          (int)(caja.medio * 2), (int)(caja.medio * 2), color);
        }
    }
};

#endif // ENTITY_H