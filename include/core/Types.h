#ifndef TYPES_H
#define TYPES_H

// Identificadores para saber qué hay dentro del Quadtree
enum TipoEntidad {
    TIPO_JUGADOR,
    TIPO_ENEMIGO,
    TIPO_BOMBA,
    TIPO_EXPLOSION,
    TIPO_MURO_INDESTRUCTIBLE,
    TIPO_MURO_DESTRUCTIBLE,
    TIPO_POWERUP
};

// Tipos de mejoras clásicas de NES
enum TipoPowerUp {
    PWR_FUEGO,      // Aumenta el rango de explosión
    PWR_BOMBA,      // Permite poner más bombas simultáneas
    PWR_VELOCIDAD   // Aumenta la velocidad de movimiento
};

struct Punto { float x, y; };

struct AABB {
    Punto centro; float medio;

    bool contiene(Punto p) {
        return (p.x >= centro.x - medio && p.x <= centro.x + medio &&
                p.y >= centro.y - medio && p.y <= centro.y + medio);
    }

    bool intersecta(AABB otro) {
        return !(otro.centro.x - otro.medio > centro.x + medio ||
                 otro.centro.x + otro.medio < centro.x - medio ||
                 otro.centro.y - otro.medio > centro.y + medio ||
                 otro.centro.y + otro.medio < centro.y - medio);
    }
};

#endif // TYPES_H