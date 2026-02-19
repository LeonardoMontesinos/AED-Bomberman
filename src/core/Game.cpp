#include "core/Game.h"
#include <stdlib.h>

// Función auxiliar en C++ puro para eliminar nodos muertos y reparar la lista
template<typename T>
void limpiarLista(ListaEnlazada<T*>& lista) {
    auto actual = lista.cabeza;
    decltype(actual) previo = nullptr;

    while (actual != nullptr) {
        if (!actual->dato->activo) {
            delete actual->dato; // Liberar la memoria del objeto Entity
            auto aBorrar = actual;

            // Desenlazar el nodo
            if (previo) previo->siguiente = actual->siguiente;
            else lista.cabeza = actual->siguiente;

            actual = actual->siguiente;
            delete aBorrar; // Liberar la memoria del Nodo de la lista
        } else {
            previo = actual;
            actual = actual->siguiente;
        }
    }
}

Game::Game(int screenWidth, int screenHeight) {
    width = screenWidth; height = screenHeight;
    InitWindow(width, height, "Bomberman Custom C++ Engine");
    SetTargetFPS(60);

    jugador = new Player(60.0f, 60.0f); // Posición inicial en la zona segura
    mapa = new Map();
    mapa->cargarMapa(1); // Carga el mapa clásico
    quadtree = nullptr;

    gameOver = false; // El juego inicia con normalidad
}

Game::~Game() {
    delete jugador;
    delete mapa;
    if (quadtree) delete quadtree;
    limpiarEntidadesInactivas();
    CloseWindow();
}

// ----------------------------------------------------------------------------------
// NUEVA IMPLEMENTACIÓN DE HANDLEINPUT (Movimiento con deslizamiento / Sliding)
// ----------------------------------------------------------------------------------
void Game::handleInput(float dt) {
    if (!jugador->activo) return;

    float vel = jugador->velocidad * dt;
    float dx = 0.0f;
    float dy = 0.0f;

    // Detectar input y asignar dirección de mirada
    if (IsKeyDown(KEY_W)) { dy -= vel; jugador->mirando = ARRIBA; }
    if (IsKeyDown(KEY_S)) { dy += vel; jugador->mirando = ABAJO; }
    if (IsKeyDown(KEY_A)) { dx -= vel; jugador->mirando = IZQUIERDA; }
    if (IsKeyDown(KEY_D)) { dx += vel; jugador->mirando = DERECHA; }

    // COLOCAR BOMBA
    if (IsKeyPressed(KEY_SPACE) && jugador->bombasActivas < jugador->maxBombas) {
        float TILE = 40.0f;
        // Centrar bomba en la grilla
        float centroX = ((int)(jugador->caja.centro.x / TILE)) * TILE + (TILE / 2.0f);
        float centroY = ((int)(jugador->caja.centro.y / TILE)) * TILE + (TILE / 2.0f);
        bombas.insertar(new Bomb(centroX, centroY, jugador->poderFuego, jugador));
        jugador->bombasActivas++;
    }

    // --- RECONSTRUIR QUADTREE ---
    if (quadtree) delete quadtree;
    quadtree = new Quadtree<Entity>(AABB{{width/2.0f, height/2.0f}, width/2.0f}, 4);

    // Inserción actualizada: Pasamos el objeto entero (según tu requerimiento)
    // NOTA: Si esto da error, usa: quadtree->insertar(m->dato->caja.centro, m->dato);
    auto m = mapa->muros.cabeza; while(m) { if(m->dato->activo) quadtree->insertar(m->dato); m = m->siguiente; }
    auto b = bombas.cabeza;      while(b) { if(b->dato->activo) quadtree->insertar(b->dato); b = b->siguiente; }
    auto e = explosiones.cabeza; while(e) { if(e->dato->activo) quadtree->insertar(e->dato); e = e->siguiente; }
    auto p = powerups.cabeza;    while(p) { if(p->dato->activo) quadtree->insertar(p->dato); p = p->siguiente; }

    // --- EVALUAR EJE X (Movimiento Horizontal) ---
    if (dx != 0.0f) {
        Punto futuroX = jugador->caja.centro;
        futuroX.x += dx;
        AABB cajaX = {futuroX, jugador->caja.medio};

        Quadtree<Entity>::Nodo* resX = nullptr;
        quadtree->consultar(cajaX, &resX);
        bool chocaX = false;

        auto actualX = resX;
        while (actualX) {
            Entity* ent = actualX->datos;

            // Colisión con Bombas (si no es la recien colocada)
            if (ent->tipo == TIPO_BOMBA && !((Bomb*)ent)->recienColocada) chocaX = true;
            // Colisión con Muros
            else if (ent->solido) chocaX = true;

            // Daño
            if (ent->tipo == TIPO_EXPLOSION) jugador->activo = false;

            // PowerUps
            if (ent->tipo == TIPO_POWERUP && ent->activo) {
                ent->activo = false;
                PowerUp* pwr = (PowerUp*)ent;
                if (pwr->tipoPoder == PWR_BOMBA) jugador->maxBombas++;
                if (pwr->tipoPoder == PWR_FUEGO) jugador->poderFuego++;
                if (pwr->tipoPoder == PWR_VELOCIDAD) jugador->velocidad += 50.0f;
            }

            auto aBorrar = actualX;
            actualX = actualX->siguiente;
            delete aBorrar;
        }

        // Si no hay colisión en X, aplicamos el movimiento
        if (!chocaX) jugador->caja.centro.x = futuroX.x;
    }

    // --- EVALUAR EJE Y (Movimiento Vertical) ---
    if (dy != 0.0f) {
        Punto futuroY = jugador->caja.centro;
        futuroY.y += dy;
        AABB cajaY = {futuroY, jugador->caja.medio};

        Quadtree<Entity>::Nodo* resY = nullptr;
        quadtree->consultar(cajaY, &resY);
        bool chocaY = false;

        auto actualY = resY;
        while (actualY) {
            Entity* ent = actualY->datos;

            if (ent->tipo == TIPO_BOMBA && !((Bomb*)ent)->recienColocada) chocaY = true;
            else if (ent->solido) chocaY = true;

            if (ent->tipo == TIPO_EXPLOSION) jugador->activo = false;

            // Nota: Los powerups también se chequean aquí por si nos movemos solo verticalmente
            if (ent->tipo == TIPO_POWERUP && ent->activo) {
                ent->activo = false;
                PowerUp* pwr = (PowerUp*)ent;
                if (pwr->tipoPoder == PWR_BOMBA) jugador->maxBombas++;
                if (pwr->tipoPoder == PWR_FUEGO) jugador->poderFuego++;
                if (pwr->tipoPoder == PWR_VELOCIDAD) jugador->velocidad += 50.0f;
            }

            auto aBorrar = actualY;
            actualY = actualY->siguiente;
            delete aBorrar;
        }

        // Si no hay colisión en Y, aplicamos el movimiento
        if (!chocaY) jugador->caja.centro.y = futuroY.y;
    }
}

void Game::generarExplosion(float centroX, float centroY, int poder) {
    float TILE = 40.0f;

    // 1. Fuego central
    explosiones.insertar(new Explosion(centroX, centroY));

    // 2. Expansión direccional (Derecha, Izquierda, Abajo, Arriba)
    float direcciones[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (int d = 0; d < 4; d++) {
        for (int i = 1; i <= poder; i++) {
            float testX = centroX + (direcciones[d][0] * TILE * i);
            float testY = centroY + (direcciones[d][1] * TILE * i);

            AABB celdaPrueba = {{testX, testY}, 2.0f};
            Quadtree<Entity>::Nodo* resultados = nullptr;
            quadtree->consultar(celdaPrueba, &resultados);

            bool detenerFuego = false;
            auto actualResult = resultados;

            while (actualResult) {
                Entity* colision = actualResult->datos;

                if (colision->tipo == TIPO_MURO_INDESTRUCTIBLE) {
                    detenerFuego = true;
                }
                else if (colision->tipo == TIPO_MURO_DESTRUCTIBLE) {
                    colision->activo = false; // Se rompe el muro
                    detenerFuego = true;      // El fuego se detiene aquí

                    // LÓGICA DE PROBABILIDAD DE POWERUPS (30%)
                    if (rand() % 100 < 30) {
                        TipoPowerUp tipoAzar;
                        int dado = rand() % 3; // 0, 1 o 2

                        if (dado == 0) tipoAzar = PWR_FUEGO;
                        else if (dado == 1) tipoAzar = PWR_BOMBA;
                        else tipoAzar = PWR_VELOCIDAD;

                        powerups.insertar(new PowerUp(testX, testY, tipoAzar));
                    }
                }
                else if (colision->tipo == TIPO_BOMBA) {
                    // Reacción en cadena
                    Bomb* otraBomba = (Bomb*)colision;
                    otraBomba->tiempoRestante = 0.0f;
                }

                auto aBorrar = actualResult;
                actualResult = actualResult->siguiente;
                delete aBorrar;
            }

            if (detenerFuego) break;
            else explosiones.insertar(new Explosion(testX, testY));
        }
    }
}

void Game::update(float dt) {
    if (gameOver) return; // Si estamos en Game Over, se congela la lógica

    handleInput(dt);
    jugador->update(dt);

    if (!jugador->activo) {
        gameOver = true; // Activa la bandera de fin de juego
    }

    auto b = bombas.cabeza;
    while(b) {
        b->dato->update(dt); // Esto actualiza 'recienColocada' si el jugador ya salió
        if (!b->dato->activo && b->dato->tiempoRestante <= 0.0f) {
            generarExplosion(b->dato->caja.centro.x, b->dato->caja.centro.y, b->dato->poderFuego);
            b->dato->tiempoRestante = -1.0f;
        }
        b = b->siguiente;
    }

    auto e = explosiones.cabeza; while(e) { e->dato->update(dt); e = e->siguiente; }

    limpiarEntidadesInactivas();
}

void Game::limpiarEntidadesInactivas() {
    limpiarLista(mapa->muros);
    limpiarLista(bombas);
    limpiarLista(explosiones);
    limpiarLista(powerups);
}

void Game::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    mapa->draw();

    // Z-Index: Dibujamos en orden para que las cosas no se superpongan raro
    auto p = powerups.cabeza;    while(p) { p->dato->draw(); p = p->siguiente; }
    auto b = bombas.cabeza;      while(b) { b->dato->draw(); b = b->siguiente; }
    auto e = explosiones.cabeza; while(e) { e->dato->draw(); e = e->siguiente; }

    // Si el jugador está vivo, lo dibujamos
    if (jugador->activo) jugador->draw();

    // Pantalla superpuesta de Game Over
    if (gameOver) {
        DrawRectangle(0, height/2 - 60, width, 120, Fade(BLACK, 0.8f));
        DrawText("GAME OVER", width/2 - 110, height/2 - 20, 40, RED);
    }

    DrawFPS(10, 10);
    EndDrawing();
}

void Game::run() {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(dt);
        render();
    }
}