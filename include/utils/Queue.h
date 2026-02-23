#ifndef QUEUE_H
#define QUEUE_H

template <typename T>
class Queue {
private:
    struct Nodo {
        T dato;
        Nodo* siguiente;
        Nodo(const T& val) : dato(val), siguiente(nullptr) {}
    };

    Nodo* frente;
    Nodo* fondo;
    int cantidad;

public:
    Queue() : frente(nullptr), fondo(nullptr), cantidad(0) {}

    ~Queue() {
        while (!empty()) pop();
    }

    void push(const T& valor) {
        Nodo* nuevo = new Nodo(valor);
        if (fondo) fondo->siguiente = nuevo;
        else frente = nuevo;
        fondo = nuevo;
        cantidad++;
    }

    void pop() {
        if (empty()) return;
        Nodo* aBorrar = frente;
        frente = frente->siguiente;
        if (!frente) fondo = nullptr;
        delete aBorrar;
        cantidad--;
    }

    T& front() {
        return frente->dato;
    }

    const T& front() const {
        return frente->dato;
    }

    bool empty() const {
        return cantidad == 0;
    }

    int size() const {
        return cantidad;
    }
};

#endif // QUEUE_H