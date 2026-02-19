#ifndef TYPES_H
#define TYPES_H

#include <cmath>

struct Punto {
    float x;
    float y;
};

struct AABB {
    Punto centro;
    float medio;

    // Verifica si chocan (se tocan)
    bool intersecta(AABB otra) const {
        if (std::abs(centro.x - otra.centro.x) > (medio + otra.medio)) return false;
        if (std::abs(centro.y - otra.centro.y) > (medio + otra.medio)) return false;
        return true;
    }

    // --- NUEVO: Verifica si la "otra" caja cabe 100% adentro de esta ---
    bool contiene(AABB otra) const {
        bool dentroX = (otra.centro.x - otra.medio >= centro.x - medio) &&
                       (otra.centro.x + otra.medio <= centro.x + medio);
        bool dentroY = (otra.centro.y - otra.medio >= centro.y - medio) &&
                       (otra.centro.y + otra.medio <= centro.y + medio);
        return dentroX && dentroY;
    }
};

enum Direccion { ARRIBA, ABAJO, IZQUIERDA, DERECHA };
enum TipoEntidad { TIPO_JUGADOR, TIPO_MURO_INDESTRUCTIBLE, TIPO_MURO_DESTRUCTIBLE, TIPO_BOMBA, TIPO_EXPLOSION, TIPO_POWERUP, TIPO_ENEMIGO };
enum TipoPowerUp { PWR_BOMBA, PWR_FUEGO, PWR_VELOCIDAD };

#endif // TYPES_H