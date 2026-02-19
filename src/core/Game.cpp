#include "core/Game.h"
#include <stdlib.h>

template<typename T>
void limpiarLista(ListaEnlazada<T*>& lista) {
    auto actual = lista.cabeza;
    decltype(actual) previo = nullptr;

    while (actual != nullptr) {
        if (!actual->dato->activo) {
            delete actual->dato;
            auto aBorrar = actual;
            if (previo) previo->siguiente = actual->siguiente;
            else lista.cabeza = actual->siguiente;
            actual = actual->siguiente;
            delete aBorrar;
        } else {
            previo = actual;
            actual = actual->siguiente;
        }
    }
}

// --- NUEVO: Función auxiliar para contar elementos en una lista ---
template<typename T>
int contarLista(const ListaEnlazada<T*>& lista) {
    int contador = 0;
    auto actual = lista.cabeza;
    while (actual) { contador++; actual = actual->siguiente; }
    return contador;
}

Game::Game(int screenWidth, int screenHeight) {
    width = screenWidth; height = screenHeight;
    InitWindow(width, height, "Bomberman Custom C++ Engine");
    SetTargetFPS(60);

    jugador = new Player(60.0f, 60.0f);
    mapa = new Map();
    mapa->cargarMapa(1);
    quadtree = nullptr;
    gameOver = false;

    totalEntidades = 0;
    colisionesComprobadas = 0;

    camara.target = {jugador->caja.centro.x, jugador->caja.centro.y};
    camara.offset = {width / 2.0f, height / 2.0f};
    camara.rotation = 0.0f;
    camara.zoom = 1.0f;
}

Game::~Game() {
    delete jugador;
    delete mapa;
    if (quadtree) delete quadtree;
    limpiarEntidadesInactivas();
    CloseWindow();
}

void Game::handleInput(float dt) {
    if (!jugador->activo) return;

    colisionesComprobadas = 0; // Reiniciamos el contador cada frame

    float vel = jugador->velocidad * dt;
    float dx = 0.0f;
    float dy = 0.0f;

    if (IsKeyDown(KEY_W)) { dy -= vel; jugador->mirando = ARRIBA; }
    if (IsKeyDown(KEY_S)) { dy += vel; jugador->mirando = ABAJO; }
    if (IsKeyDown(KEY_A)) { dx -= vel; jugador->mirando = IZQUIERDA; }
    if (IsKeyDown(KEY_D)) { dx += vel; jugador->mirando = DERECHA; }

    if (IsKeyPressed(KEY_SPACE) && jugador->bombasActivas < jugador->maxBombas) {
        float TILE = 40.0f;
        float centroX = ((int)(jugador->caja.centro.x / TILE)) * TILE + (TILE / 2.0f);
        float centroY = ((int)(jugador->caja.centro.y / TILE)) * TILE + (TILE / 2.0f);
        bombas.insertar(new Bomb(centroX, centroY, jugador->poderFuego, jugador));
        jugador->bombasActivas++;
    }

    if (quadtree) delete quadtree;
    quadtree = new Quadtree<Entity>(AABB{{800.0f, 800.0f}, 800.0f}, 4);

    auto m = mapa->muros.cabeza; while(m) { if(m->dato->activo) quadtree->insertar(m->dato); m = m->siguiente; }
    auto b = bombas.cabeza;      while(b) { if(b->dato->activo) quadtree->insertar(b->dato); b = b->siguiente; }
    auto e = explosiones.cabeza; while(e) { if(e->dato->activo) quadtree->insertar(e->dato); e = e->siguiente; }
    auto p = powerups.cabeza;    while(p) { if(p->dato->activo) quadtree->insertar(p->dato); p = p->siguiente; }

    if (dx != 0.0f) {
        Punto futuroX = jugador->caja.centro; futuroX.x += dx;
        AABB cajaX = {futuroX, jugador->caja.medio};

        Quadtree<Entity>::Nodo* resX = nullptr;
        quadtree->consultar(cajaX, &resX);
        bool chocaX = false;

        auto actualX = resX;
        while (actualX) {
            colisionesComprobadas++; // SUMAMOS UNA COMPROBACIÓN

            Entity* ent = actualX->datos;
            if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaX = true; }
            else if (ent->solido) { chocaX = true; }

            if (ent->tipo == TIPO_EXPLOSION) jugador->activo = false;
            if (ent->tipo == TIPO_POWERUP && ent->activo) {
                ent->activo = false;
                PowerUp* pwr = (PowerUp*)ent;
                if (pwr->tipoPoder == PWR_BOMBA) jugador->maxBombas++;
                if (pwr->tipoPoder == PWR_FUEGO) jugador->poderFuego++;
                if (pwr->tipoPoder == PWR_VELOCIDAD) jugador->velocidad += 50.0f;
            }
            auto aBorrar = actualX; actualX = actualX->siguiente; delete aBorrar;
        }
        if (!chocaX) jugador->caja.centro.x = futuroX.x;
    }

    if (dy != 0.0f) {
        Punto futuroY = jugador->caja.centro; futuroY.y += dy;
        AABB cajaY = {futuroY, jugador->caja.medio};

        Quadtree<Entity>::Nodo* resY = nullptr;
        quadtree->consultar(cajaY, &resY);
        bool chocaY = false;

        auto actualY = resY;
        while (actualY) {
            colisionesComprobadas++; // SUMAMOS UNA COMPROBACIÓN

            Entity* ent = actualY->datos;
            if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaY = true; }
            else if (ent->solido) { chocaY = true; }

            if (ent->tipo == TIPO_EXPLOSION) jugador->activo = false;
            if (ent->tipo == TIPO_POWERUP && ent->activo) {
                ent->activo = false;
                PowerUp* pwr = (PowerUp*)ent;
                if (pwr->tipoPoder == PWR_BOMBA) jugador->maxBombas++;
                if (pwr->tipoPoder == PWR_FUEGO) jugador->poderFuego++;
                if (pwr->tipoPoder == PWR_VELOCIDAD) jugador->velocidad += 50.0f;
            }
            auto aBorrar = actualY; actualY = actualY->siguiente; delete aBorrar;
        }
        if (!chocaY) jugador->caja.centro.y = futuroY.y;
    }
}

void Game::generarExplosion(float centroX, float centroY, int poder) {
    float TILE = 40.0f;
    explosiones.insertar(new Explosion(centroX, centroY));
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
                colisionesComprobadas++; // SUMAMOS COMPROBACIÓN DEL FUEGO

                Entity* colision = actualResult->datos;
                if (colision->tipo == TIPO_MURO_INDESTRUCTIBLE) detenerFuego = true;
                else if (colision->tipo == TIPO_MURO_DESTRUCTIBLE) {
                    colision->activo = false;
                    detenerFuego = true;
                    if (rand() % 100 < 30) {
                        TipoPowerUp tipoAzar;
                        int dado = rand() % 3;
                        if (dado == 0) tipoAzar = PWR_FUEGO;
                        else if (dado == 1) tipoAzar = PWR_BOMBA;
                        else tipoAzar = PWR_VELOCIDAD;
                        powerups.insertar(new PowerUp(testX, testY, tipoAzar));
                    }
                }
                else if (colision->tipo == TIPO_BOMBA) {
                    Bomb* otraBomba = (Bomb*)colision;
                    otraBomba->tiempoRestante = 0.0f;
                }
                auto aBorrar = actualResult; actualResult = actualResult->siguiente; delete aBorrar;
            }
            if (detenerFuego) break;
            else explosiones.insertar(new Explosion(testX, testY));
        }
    }
}

void Game::update(float dt) {
    if (gameOver) return;

    handleInput(dt);
    jugador->update(dt);

    camara.target = {jugador->caja.centro.x, jugador->caja.centro.y};

    if (!jugador->activo) gameOver = true;

    auto b = bombas.cabeza;
    while(b) {
        b->dato->update(dt);
        if (!b->dato->activo && b->dato->tiempoRestante <= 0.0f) {
            generarExplosion(b->dato->caja.centro.x, b->dato->caja.centro.y, b->dato->poderFuego);
            b->dato->tiempoRestante = -1.0f;
        }
        b = b->siguiente;
    }

    auto e = explosiones.cabeza; while(e) { e->dato->update(dt); e = e->siguiente; }

    limpiarEntidadesInactivas();

    totalEntidades = contarLista(mapa->muros) + contarLista(bombas) + contarLista(explosiones) + contarLista(powerups) + 1; // +1 del Jugador
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

    BeginMode2D(camara);

    mapa->draw();
    auto p = powerups.cabeza;    while(p) { p->dato->draw(); p = p->siguiente; }
    auto b = bombas.cabeza;      while(b) { b->dato->draw(); b = b->siguiente; }
    auto e = explosiones.cabeza; while(e) { e->dato->draw(); e = e->siguiente; }

    if (jugador->activo) jugador->draw();
    if (quadtree) quadtree->draw();

    EndMode2D();


    int hudX = width - 230;
    DrawRectangle(hudX, 10, 220, 140, Fade(BLACK, 0.7f));
    DrawRectangleLines(hudX, 10, 220, 140, LIGHTGRAY); // Un borde bonito

    DrawText("RENDIMIENTO QUADTREE", hudX + 10, 20, 10, GREEN);
    DrawText(TextFormat("Total Entidades: %d", totalEntidades), hudX + 10, 35, 10, WHITE);
    DrawText(TextFormat("Comprobaciones: %d", colisionesComprobadas), hudX + 10, 50, 10, RED);

    DrawText("ESTADISTICAS JUGADOR", hudX + 10, 80, 10, YELLOW);
    DrawText(TextFormat("Bombas Max: %d", jugador->maxBombas), hudX + 10, 95, 10, WHITE);
    DrawText(TextFormat("Fuego Rango: %d", jugador->poderFuego), hudX + 10, 110, 10, WHITE);
    DrawText(TextFormat("Velocidad: %d", (int)jugador->velocidad), hudX + 10, 125, 10, WHITE);

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