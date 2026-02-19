#ifndef QUADTREE_H
#define QUADTREE_H

#include <raylib.h> // NUEVO: Necesario para dibujar las líneas
#include "core/Types.h"

template <typename T>
class Quadtree {
private:
    AABB limite;
    int capacidad;
    bool dividido;
    Quadtree* noroeste; Quadtree* noreste;
    Quadtree* suroeste; Quadtree* sureste;

public:
    struct Nodo {
        T* datos;
        Nodo* siguiente;
    };
private:
    Nodo* objetos;

public:
    Quadtree(AABB limite, int capacidad) : limite(limite), capacidad(capacidad), dividido(false), noroeste(nullptr), noreste(nullptr), suroeste(nullptr), sureste(nullptr), objetos(nullptr) {}

    ~Quadtree() { limpiar(); }

    void subdividir() {
        float x = limite.centro.x; float y = limite.centro.y; float h = limite.medio / 2;
        noroeste = new Quadtree({{x - h, y - h}, h}, capacidad);
        noreste = new Quadtree({{x + h, y - h}, h}, capacidad);
        suroeste = new Quadtree({{x - h, y + h}, h}, capacidad);
        sureste = new Quadtree({{x + h, y + h}, h}, capacidad);
        dividido = true;
    }

    bool insertar(T* data) {
        // 1. Si ni siquiera toca este cuadrante, lo ignoramos
        if (!limite.intersecta(data->caja)) return false;

        // 2. Si ya estamos divididos, intentamos pasarlo a los hijos
        if (dividido) {
            // SOLO lo pasamos si cabe COMPLETAMENTE dentro del hijo
            if (noroeste->limite.contiene(data->caja)) return noroeste->insertar(data);
            if (noreste->limite.contiene(data->caja)) return noreste->insertar(data);
            if (suroeste->limite.contiene(data->caja)) return suroeste->insertar(data);
            if (sureste->limite.contiene(data->caja)) return sureste->insertar(data);

            // Si el objeto está justo en la línea divisoria (no cabe 100% en ningún hijo),
            // ignoramos el límite de capacidad y lo guardamos aquí en el padre.
            Nodo* nuevo = new Nodo{data, objetos};
            objetos = nuevo;
            return true;
        }

        // 3. Contamos cuántos objetos hay actualmente en este nodo
        int contador = 0;
        Nodo* temp = objetos;
        while (temp != nullptr) { contador++; temp = temp->siguiente; }

        // 4. Si hay espacio, lo guardamos
        if (contador < capacidad) {
            Nodo* nuevo = new Nodo{data, objetos};
            objetos = nuevo;
            return true;
        }

        // 5. Si está lleno, subdividimos la zona
        subdividir();

        // 6. Ahora que nos dividimos, intentamos meter el objeto nuevo en los hijos
        if (noroeste->limite.contiene(data->caja)) return noroeste->insertar(data);
        if (noreste->limite.contiene(data->caja)) return noreste->insertar(data);
        if (suroeste->limite.contiene(data->caja)) return suroeste->insertar(data);
        if (sureste->limite.contiene(data->caja)) return sureste->insertar(data);

        // Si llegó hasta aquí, significa que el objeto cruza la línea roja central.
        // Como no cabe entero en ningún hijo, se queda a vivir en este cuadrante padre.
        Nodo* nuevo = new Nodo{data, objetos};
        objetos = nuevo;
        return true;
    }

    void consultar(AABB rango, Nodo** listaResultados) {
        if (!limite.intersecta(rango)) return;

        Nodo* actual = objetos;
        while (actual != nullptr) {
            if (rango.intersecta(actual->datos->caja)) {
                Nodo* resultado = new Nodo{actual->datos, *listaResultados};
                *listaResultados = resultado;
            }
            actual = actual->siguiente;
        }

        if (dividido) {
            noroeste->consultar(rango, listaResultados); noreste->consultar(rango, listaResultados);
            suroeste->consultar(rango, listaResultados); sureste->consultar(rango, listaResultados);
        }
    }

    void limpiar() {
        Nodo* actual = objetos;
        while (actual != nullptr) {
            Nodo* aBorrar = actual; actual = actual->siguiente; delete aBorrar;
        }
        objetos = nullptr;
        if (dividido) {
            delete noroeste; delete noreste; delete suroeste; delete sureste;
            dividido = false;
        }
    }

    void draw() {
        DrawRectangleLines(
            (int)(limite.centro.x - limite.medio),
            (int)(limite.centro.y - limite.medio),
            (int)(limite.medio * 2),
            (int)(limite.medio * 2),
            RED
        );

        if (dividido) {
            noroeste->draw();
            noreste->draw();
            suroeste->draw();
            sureste->draw();
        }
    }
};

#endif // QUADTREE_H