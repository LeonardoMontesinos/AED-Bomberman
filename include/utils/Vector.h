#ifndef VECTOR_H
#define VECTOR_H

template <typename T>
class Vector {
private:
    T* datos;       // Puntero al bloque de memoria en el Heap
    int cantidad;   // Cantidad de elementos actuales

public:
    Vector() : datos(nullptr), cantidad(0) {}

    ~Vector() {
        if (datos != nullptr) {
            delete[] datos;
        }
    }

    void resize(int nuevaCantidad) {
        if (datos != nullptr) {
            delete[] datos;
        }

        cantidad = nuevaCantidad;

        if (cantidad > 0) {
            datos = new T[cantidad]();
        } else {
            datos = nullptr;
        }
    }

    int size() const {
        return cantidad;
    }

    bool empty() const {
        return cantidad == 0;
    }

    T& operator[](int index) {
        return datos[index];
    }

	// solo lectura
    const T& operator[](int index) const {
        return datos[index];
    }
};

#endif // VECTOR_H