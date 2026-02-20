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

Game::Game(int screenWidth, int screenHeight) {
    estadoActual = MENU_PRINCIPAL;
    opcionMenu = 0;
    debugMode = false;
    width = screenWidth; height = screenHeight;
    InitWindow(width, height, "Bomberman Custom C++ Engine");
    InitAudioDevice();
    fxExplosion = LoadSound("assets/explosion.wav");
    fxVictoria = LoadSound("assets/victoria.wav");
    fxDerrota = LoadSound("assets/derrota.wav");
    fxPickUp = LoadSound("assets/powerup.wav");
    ganador = 0;
    SetTargetFPS(60);
    estadoActual = MENU_PRINCIPAL;
    jugador = new Player(60.0f, 60.0f);
    jugador2 = new Player(width - 60.0f, height - 60.0f);
    mapa = new Map();
    mapa->cargarMapa(1);
    quadtree = nullptr;
    gameOver = false;
}

Game::~Game() {
    UnloadSound(fxExplosion);
    UnloadSound(fxVictoria);
    UnloadSound(fxPickUp);
    CloseAudioDevice();
    delete jugador;
    delete jugador2;
    delete mapa;
    if (quadtree) delete quadtree;
    limpiarEntidadesInactivas();
    CloseWindow();
}

void Game::handleInput(float dt) {
    float dx1 = 0.0f, dy1 = 0.0f;
    float dx2 = 0.0f, dy2 = 0.0f;

    // Inputs Jugador 1
    if (jugador->activo) {
        float vel = jugador->velocidad * dt;
        if (IsKeyDown(KEY_W)) { dy1 -= vel; jugador->mirando = ARRIBA; }
        if (IsKeyDown(KEY_S)) { dy1 += vel; jugador->mirando = ABAJO; }
        if (IsKeyDown(KEY_A)) { dx1 -= vel; jugador->mirando = IZQUIERDA; }
        if (IsKeyDown(KEY_D)) { dx1 += vel; jugador->mirando = DERECHA; }

        if (IsKeyPressed(KEY_SPACE) && jugador->bombasActivas < jugador->maxBombas) {
            float TILE = 40.0f;
            float centroX = ((int)(jugador->caja.centro.x / TILE)) * TILE + (TILE / 2.0f);
            float centroY = ((int)(jugador->caja.centro.y / TILE)) * TILE + (TILE / 2.0f);
            bombas.insertar(new Bomb(centroX, centroY, jugador->poderFuego, jugador));
            jugador->bombasActivas++;
        }
    }

    // Inputs Jugador 2
    if (estadoActual == PVP && jugador2->activo) {
        float vel2 = jugador2->velocidad * dt;
        if (IsKeyDown(KEY_UP)) { dy2 -= vel2; jugador2->mirando = ARRIBA; }
        if (IsKeyDown(KEY_DOWN)) { dy2 += vel2; jugador2->mirando = ABAJO; }
        if (IsKeyDown(KEY_LEFT)) { dx2 -= vel2; jugador2->mirando = IZQUIERDA; }
        if (IsKeyDown(KEY_RIGHT)) { dx2 += vel2; jugador2->mirando = DERECHA; }

        if (IsKeyPressed(KEY_ENTER) && jugador2->bombasActivas < jugador2->maxBombas) {
            float TILE = 40.0f;
            float centroX = ((int)(jugador2->caja.centro.x / TILE)) * TILE + (TILE / 2.0f);
            float centroY = ((int)(jugador2->caja.centro.y / TILE)) * TILE + (TILE / 2.0f);
            bombas.insertar(new Bomb(centroX, centroY, jugador2->poderFuego, jugador2));
            jugador2->bombasActivas++;
        }
    }

    if (quadtree) delete quadtree;
    quadtree = new Quadtree<Entity>(AABB{{width/2.0f, height/2.0f}, width/2.0f}, 4);
    auto m = mapa->muros.cabeza; while(m) { if(m->dato->activo) quadtree->insertar(m->dato); m = m->siguiente; }
    auto b = bombas.cabeza;      while(b) { if(b->dato->activo) quadtree->insertar(b->dato); b = b->siguiente; }
    auto e = explosiones.cabeza; while(e) { if(e->dato->activo) quadtree->insertar(e->dato); e = e->siguiente; }
    auto p = powerups.cabeza;    while(p) { if(p->dato->activo) quadtree->insertar(p->dato); p = p->siguiente; }

    // Manejo de Colisiones Jugador 1
    if (jugador->activo) {
        if (dx1 != 0.0f) {
            Punto futuroX = jugador->caja.centro; futuroX.x += dx1; AABB cajaX = {futuroX, jugador->caja.medio};
            Quadtree<Entity>::Nodo* resX = nullptr; quadtree->consultar(cajaX, &resX);
            bool chocaX = false; auto actualX = resX;
            while (actualX) {
                Entity* ent = actualX->datos;
                if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaX = true; }
                else if (ent->solido) chocaX = true;
                if (ent->tipo == TIPO_EXPLOSION) jugador->activo = false;
                if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(jugador, ent);
                auto aBorrar = actualX; actualX = actualX->siguiente; delete aBorrar;
            }
            if (!chocaX) jugador->caja.centro.x = futuroX.x;
        }

        if (dy1 != 0.0f) {
            Punto futuroY = jugador->caja.centro; futuroY.y += dy1; AABB cajaY = {futuroY, jugador->caja.medio};
            Quadtree<Entity>::Nodo* resY = nullptr; quadtree->consultar(cajaY, &resY);
            bool chocaY = false; auto actualY = resY;
            while (actualY) {
                Entity* ent = actualY->datos;
                if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaY = true; }
                else if (ent->solido) chocaY = true;
                if (ent->tipo == TIPO_EXPLOSION) jugador->activo = false;
                if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(jugador, ent);

                auto aBorrar = actualY; actualY = actualY->siguiente; delete aBorrar;
            }
            if (!chocaY) jugador->caja.centro.y = futuroY.y;
        }

        Quadtree<Entity>::Nodo* resImpacto = nullptr; quadtree->consultar(jugador->caja, &resImpacto); auto actImp = resImpacto;
        while (actImp) {
            Entity* entImp = actImp->datos;
            if (entImp->tipo == TIPO_EXPLOSION) jugador->activo = false;
            if (entImp->tipo == TIPO_POWERUP && entImp->activo) aplicarPowerUp(jugador, entImp);

            auto aBorrar = actImp; actImp = actImp->siguiente; delete aBorrar;
        }
    }

    // Manejo de Colisiones Jugador 2
    if (estadoActual == PVP && jugador2->activo) {
        if (dx2 != 0.0f) {
            Punto futuroX = jugador2->caja.centro; futuroX.x += dx2; AABB cajaX = {futuroX, jugador2->caja.medio};
            Quadtree<Entity>::Nodo* resX = nullptr; quadtree->consultar(cajaX, &resX);
            bool chocaX = false; auto actualX = resX;
            while (actualX) {
                Entity* ent = actualX->datos;
                if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaX = true; }
                else if (ent->solido) chocaX = true;
                if (ent->tipo == TIPO_EXPLOSION) jugador2->activo = false;
                if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(jugador2, ent);
                auto aBorrar = actualX; actualX = actualX->siguiente; delete aBorrar;
            }
            if (!chocaX) jugador2->caja.centro.x = futuroX.x;
        }

        if (dy2 != 0.0f) {
            Punto futuroY = jugador2->caja.centro; futuroY.y += dy2; AABB cajaY = {futuroY, jugador2->caja.medio};
            Quadtree<Entity>::Nodo* resY = nullptr; quadtree->consultar(cajaY, &resY);
            bool chocaY = false; auto actualY = resY;
            while (actualY) {
                Entity* ent = actualY->datos;
                if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaY = true; }
                else if (ent->solido) chocaY = true;
                if (ent->tipo == TIPO_EXPLOSION) jugador2->activo = false;
                if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(jugador2, ent);
                auto aBorrar = actualY; actualY = actualY->siguiente; delete aBorrar;
            }
            if (!chocaY) jugador2->caja.centro.y = futuroY.y;
        }

        Quadtree<Entity>::Nodo* resImpacto = nullptr; quadtree->consultar(jugador2->caja, &resImpacto); auto actImp = resImpacto;
        while (actImp) {
            Entity* entImp = actImp->datos;
            if (entImp->tipo == TIPO_EXPLOSION) jugador2->activo = false;
            if (entImp->tipo == TIPO_POWERUP && entImp->activo) aplicarPowerUp(jugador2, entImp);
            auto aBorrar = actImp; actImp = actImp->siguiente; delete aBorrar;
        }
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
                Entity* colision = actualResult->datos;

                if (colision->tipo == TIPO_MURO_INDESTRUCTIBLE) {
                    detenerFuego = true;
                }
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
    if (IsKeyPressed(KEY_F3)) debugMode = !debugMode;

    if (estadoActual == MENU_PRINCIPAL) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) { opcionMenu--; if (opcionMenu < 0) opcionMenu = 2; }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { opcionMenu++; if (opcionMenu > 2) opcionMenu = 0; }
        if (IsKeyPressed(KEY_ENTER)) {
            if (opcionMenu == 0) estadoActual = PVE;
            else if (opcionMenu == 1) estadoActual = PVP;
            else if (opcionMenu == 2) estadoActual = PANTALLA_INFO;
        }
        return;
    }

    if (estadoActual == PANTALLA_INFO) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) estadoActual = MENU_PRINCIPAL;
        return;
    }

    if (gameOver) {
        if (IsKeyPressed(KEY_ENTER)) {
            estadoActual = MENU_PRINCIPAL;
            gameOver = false;
            ganador = 0;

            // Reiniciar Jugador 1
            jugador->activo = true; jugador->caja.centro.x = 60.0f; jugador->caja.centro.y = 60.0f;
            jugador->maxBombas = 1; jugador->bombasActivas = 0; jugador->poderFuego = 1; jugador->velocidad = 150.0f;
            jugador->timerFuego = 0.0f; jugador->timerVelocidad = 0.0f;

            // Reiniciar Jugador 2
            jugador2->activo = true; jugador2->caja.centro.x = width - 60.0f; jugador2->caja.centro.y = height - 60.0f;
            jugador2->maxBombas = 1; jugador2->bombasActivas = 0; jugador2->poderFuego = 1; jugador2->velocidad = 150.0f;
            jugador2->timerFuego = 0.0f; jugador2->timerVelocidad = 0.0f;

            bombas.limpiar(); explosiones.limpiar(); powerups.limpiar();
            mapa->cargarMapa(1);
        }
        return;
    }

    // Tiempo de Power Ups
    if (jugador->timerFuego > 0.0f) {
        jugador->timerFuego -= dt;
        if (jugador->timerFuego <= 0.0f) jugador->poderFuego = 1;
    }
    if (jugador->timerVelocidad > 0.0f) {
        jugador->timerVelocidad -= dt;
        if (jugador->timerVelocidad <= 0.0f) jugador->velocidad = 150.0f;
    }

    if (estadoActual == PVP && jugador2->activo) {
        if (jugador2->timerFuego > 0.0f) {
            jugador2->timerFuego -= dt;
            if (jugador2->timerFuego <= 0.0f) jugador2->poderFuego = 1;
        }
        if (jugador2->timerVelocidad > 0.0f) {
            jugador2->timerVelocidad -= dt;
            if (jugador2->timerVelocidad <= 0.0f) jugador2->velocidad = 150.0f;
        }
    }

    handleInput(dt);

    if (jugador->activo) jugador->update(dt);
    if (estadoActual == PVP && jugador2->activo) jugador2->update(dt);

    // Evaluar Victoria o Derrota
    if (!gameOver) {
        if (estadoActual == PVE && !jugador->activo) {
            gameOver = true;
            ganador = 2;
        }
        else if (estadoActual == PVP && (!jugador->activo || !jugador2->activo)) {
            gameOver = true;
            if (!jugador->activo && !jugador2->activo) ganador = 0; // Empate
            else if (!jugador->activo) ganador = 2; // Gana J2
            else ganador = 1; // Gana J1

            PlaySound(fxVictoria);
        }
    }

    auto b = bombas.cabeza;
    while(b) {
        b->dato->update(dt);
        if (!b->dato->activo && b->dato->tiempoRestante <= 0.0f) {
            PlaySound(fxExplosion);
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

    if (estadoActual == MENU_PRINCIPAL) {
        DrawText("QUAD TREE - BOMBERMAN", width/2 - 275, height/4, 40, BLACK);
        Color colorPVE = (opcionMenu == 0) ? RED : DARKGRAY;
        Color colorPVP = (opcionMenu == 1) ? RED : DARKGRAY;
        Color colorInfo = (opcionMenu == 2) ? RED : DARKGRAY;

        DrawText((opcionMenu == 0 ? "-> 1 Jugador vs Bot" : "   1 Jugador vs Bot"), width/2 - 140, height/2, 20, colorPVE);
        DrawText((opcionMenu == 1 ? "-> Modo Versus" : "   Modo Versus"), width/2 - 140, height/2 + 40, 20, colorPVP);
        DrawText((opcionMenu == 2 ? "-> Info y Controles" : "   Info y Controles"), width/2 - 140, height/2 + 80, 20, colorInfo);
    }
    else if (estadoActual == PANTALLA_INFO) {
        DrawText("CONTROLES Y PODERES", width/2 - 180, 50, 30, BLACK);
        DrawText("Jugador 1: WASD para mover, ESPACIO para poner una bomba", 50, 150, 20, DARKGRAY);
        DrawText("Jugador 2: Flechas para mover, ENTER para poner una bomba", 50, 190, 20, DARKGRAY);

        DrawText("Power-Ups:", 50, 250, 20, RED);
        DrawText("- Fuego: Rango +1 bloque (Por 15 segundos)", 50, 290, 20, DARKGRAY);
        DrawText("- Bomba: +1 bomba extra (Permanente)", 50, 330, 20, DARKGRAY);
        DrawText("- Velocidad: Velocidad + 50% (Por 15 segundos)", 50, 370, 20, DARKGRAY);

        DrawText("Presiona ENTER para volver", width/2 - 180, height - 50, 20, BLACK);
    }
    else if (estadoActual == PVP || estadoActual == PVE) {
        mapa->draw();

        auto p = powerups.cabeza;    while(p) { p->dato->draw(); p = p->siguiente; }
        auto b = bombas.cabeza;      while(b) { b->dato->draw(); b = b->siguiente; }
        auto e = explosiones.cabeza; while(e) { e->dato->draw(); e = e->siguiente; }

        if (jugador->activo) jugador->draw();
        if (estadoActual == PVP && jugador2->activo) jugador2->draw();

        // HUD Jugador 1
        DrawText("JUGADOR 1", 20, 10, 20, BLACK);
        int offsetX1 = 20;
        if (jugador->timerFuego > 0) {
            DrawRectangle(offsetX1, 35, 20, 20, RED); DrawText("F", offsetX1 + 5, 37, 16, WHITE); offsetX1 += 25;
        }
        if (jugador->timerVelocidad > 0) {
            DrawRectangle(offsetX1, 35, 20, 20, BLUE); DrawText("V", offsetX1 + 5, 37, 16, WHITE);
        }

        // HUD Jugador 2 (Solo si el modo es PVP)
        if (estadoActual == PVP) {
            DrawText("JUGADOR 2", width - 130, 10, 20, BLACK);
            int offsetX2 = width - 40;
            if (jugador2->timerVelocidad > 0) {
                DrawRectangle(offsetX2, 35, 20, 20, BLUE); DrawText("V", offsetX2 + 5, 37, 16, WHITE); offsetX2 -= 25;
            }
            if (jugador2->timerFuego > 0) {
                DrawRectangle(offsetX2, 35, 20, 20, RED); DrawText("F", offsetX2 + 5, 37, 16, WHITE);
            }
        }

        if (debugMode && quadtree != nullptr) {
            quadtree->drawDebug();
            DrawRectangleLines(jugador->caja.centro.x - jugador->caja.medio, jugador->caja.centro.y - jugador->caja.medio, jugador->caja.medio * 2, jugador->caja.medio * 2, BLUE);
            if (estadoActual == PVP) DrawRectangleLines(jugador2->caja.centro.x - jugador2->caja.medio, jugador2->caja.centro.y - jugador2->caja.medio, jugador2->caja.medio * 2, jugador2->caja.medio * 2, RED);
        }

        // Pantalla Game Over
        if (gameOver) {
            DrawRectangle(0, height/2 - 80, width, 180, Fade(BLACK, 0.8f));

            if (estadoActual == PVP) {
                if (ganador == 1) DrawText("¡JUGADOR 1 GANA!", width/2 - 180, height/2 - 50, 40, GREEN);
                else if (ganador == 2) DrawText("¡JUGADOR 2 GANA!", width/2 - 180, height/2 - 50, 40, BLUE);
                else DrawText("¡EMPATE!", width/2 - 90, height/2 - 50, 40, YELLOW);
            } else {
                DrawText("DERROTA...", width/2 - 110, height/2 - 50, 40, RED);
            }

            DrawText("Presiona ENTER para volver al menu", width/2 - 190, height/2 + 20, 20, LIGHTGRAY);
        }
    }

    if (debugMode) DrawText("DEBUG MODE ON", 10, height - 30, 20, RED);
    DrawFPS(width - 90, height - 30); // Moví los FPS abajo para que no estorben al HUD
    EndDrawing();
}

void Game::aplicarPowerUp(Player* jug, Entity* ent) {
    ent->activo = false;
    PowerUp* pwr = (PowerUp*)ent;
    PlaySound(fxPickUp);

    if (pwr->tipoPoder == PWR_BOMBA) {
        jug->maxBombas++;
    }
    else if (pwr->tipoPoder == PWR_FUEGO) {
        jug->poderFuego++;
        jug->timerFuego = 15.0f;
    }
    else if (pwr->tipoPoder == PWR_VELOCIDAD) {
        jug->velocidad += 50.0f;
        jug->timerVelocidad = 15.0f;
    }
}

void Game::run() {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(dt);
        render();
    }
}