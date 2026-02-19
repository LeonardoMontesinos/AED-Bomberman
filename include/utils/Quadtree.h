#ifndef QUADTREE_H
#define QUADTREE_H

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
        if (!limite.intersecta(data->caja)) return false;

        int contador = 0;
        Nodo* temp = objetos;
        while (temp != nullptr) { contador++; temp = temp->siguiente; }

        if (contador < capacidad && !dividido) {
            Nodo* nuevo = new Nodo{data, objetos};
            objetos = nuevo;
            return true;
        }

        if (!dividido) subdividir();

        // Intentamos insertarlo en todos los que toque.
        bool insertado = false;
        if (noroeste->insertar(data)) insertado = true;
        if (noreste->insertar(data)) insertado = true;
        if (suroeste->insertar(data)) insertado = true;
        if (sureste->insertar(data)) insertado = true;

        return insertado;
    }

    void consultar(AABB rango, Nodo** listaResultados) {
        // Si el rango de búsqueda no toca este cuadrante, salimos rápido
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
};

#endif // QUADTREE_H