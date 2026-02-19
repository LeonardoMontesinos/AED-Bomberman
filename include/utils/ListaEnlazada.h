#ifndef LISTA_ENLAZADA_H
#define LISTA_ENLAZADA_H

template <typename T>
class ListaEnlazada {
public:
    struct Nodo {
        T dato;
        Nodo* siguiente;
    };

    Nodo* cabeza;

    ListaEnlazada() : cabeza(nullptr) {}

    ~ListaEnlazada() { limpiar(); }

    void insertar(T elemento) {
        Nodo* nuevo = new Nodo{elemento, cabeza};
        cabeza = nuevo;
    }

    void limpiar() {
        Nodo* actual = cabeza;
        while (actual != nullptr) {
            Nodo* aBorrar = actual;
            actual = actual->siguiente;
            delete aBorrar;
        }
        cabeza = nullptr;
    }
};

#endif // LISTA_ENLAZADA_H