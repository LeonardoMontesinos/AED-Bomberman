#ifndef MAP_H
#define MAP_H

#include "utils/ListaEnlazada.h"
#include "entities/Entity.h"

class Map {
public:
    ListaEnlazada<Entity*> muros;

    Map();
    ~Map();

    // Función principal que elije qué mapa cargar
    void cargarMapa(int tipo);
    void draw();

private:
    // Generadores específicos
    void generarMapaClasico();
    void generarMapaArena();
};

#endif // MAP_H