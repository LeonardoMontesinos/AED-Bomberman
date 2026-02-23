#include "core/Game.h"
#include <stdlib.h>
#include <functional>

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
    fxPickUp = LoadSound("assets/powerup.wav");
    ganador = 0;
    SetTargetFPS(60);
    estadoActual = MENU_PRINCIPAL;
    jugador = new Player(60.0f, 60.0f);
    jugador2 = new Player(width - 60.0f, height - 60.0f);
    bot = new Player(width - 60.0f, height - 60.0f);
    bot->activo = false;
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
    delete bot;
    delete mapa;
    if (quadtree) delete quadtree;
    limpiarEntidadesInactivas();
    CloseWindow();
}

void Game::handleInput(float dt) {
    float dx1 = 0.0f, dy1 = 0.0f;
    float dx2 = 0.0f, dy2 = 0.0f;

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

    moverYColisionar(jugador, dx1, dy1);
    if (estadoActual == PVP) moverYColisionar(jugador2, dx2, dy2);

    if (estadoActual == PVE && bot->activo) {
        float dxBot = 0.0f, dyBot = 0.0f;
        bool botPoneBomba = false;
        pensarBot(dt, dxBot, dyBot, botPoneBomba);
        moverYColisionar(bot, dxBot, dyBot);

        if (botPoneBomba && bot->bombasActivas < bot->maxBombas) {
            float TILE = 40.0f;
            float centroX = ((int)(bot->caja.centro.x / TILE)) * TILE + (TILE / 2.0f);
            float centroY = ((int)(bot->caja.centro.y / TILE)) * TILE + (TILE / 2.0f);
            bombas.insertar(new Bomb(centroX, centroY, bot->poderFuego, bot));
            bot->bombasActivas++;
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
            if (opcionMenu == 0) { estadoActual = PVE; bot->activo = true; jugador2->activo = false; } // Activa BOT
            else if (opcionMenu == 1) { estadoActual = PVP; jugador2->activo = true; bot->activo = false; } // Activa J2
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
            estadoActual = MENU_PRINCIPAL; gameOver = false; ganador = 0;
            jugador->activo = true; jugador->caja.centro.x = 60.0f; jugador->caja.centro.y = 60.0f;
            jugador->maxBombas = 1; jugador->bombasActivas = 0; jugador->poderFuego = 1; jugador->velocidad = 150.0f; jugador->timerFuego = 0.0f; jugador->timerVelocidad = 0.0f;
            jugador2->caja.centro.x = width - 60.0f; jugador2->caja.centro.y = height - 60.0f; jugador2->maxBombas = 1; jugador2->bombasActivas = 0; jugador2->poderFuego = 1; jugador2->velocidad = 150.0f; jugador2->timerFuego = 0.0f; jugador2->timerVelocidad = 0.0f;
            bot->caja.centro.x = width - 60.0f; bot->caja.centro.y = height - 60.0f; bot->maxBombas = 1; bot->bombasActivas = 0; bot->poderFuego = 1; bot->velocidad = 120.0f; bot->timerFuego = 0.0f; bot->timerVelocidad = 0.0f; // Velocidad del bot reducida a 120 para balance
            bombas.limpiar(); explosiones.limpiar(); powerups.limpiar(); mapa->cargarMapa(1);
        }
        return;
    }

    if (jugador->timerFuego > 0.0f) { jugador->timerFuego -= dt; if (jugador->timerFuego <= 0.0f) jugador->poderFuego = 1; }
    if (jugador->timerVelocidad > 0.0f) { jugador->timerVelocidad -= dt; if (jugador->timerVelocidad <= 0.0f) jugador->velocidad = 150.0f; }

    if (estadoActual == PVP && jugador2->activo) {
        if (jugador2->timerFuego > 0.0f) { jugador2->timerFuego -= dt; if (jugador2->timerFuego <= 0.0f) jugador2->poderFuego = 1; }
        if (jugador2->timerVelocidad > 0.0f) { jugador2->timerVelocidad -= dt; if (jugador2->timerVelocidad <= 0.0f) jugador2->velocidad = 150.0f; }
    }

    if (estadoActual == PVE && bot->activo) {
        if (bot->timerFuego > 0.0f) { bot->timerFuego -= dt; if (bot->timerFuego <= 0.0f) bot->poderFuego = 1; }
        if (bot->timerVelocidad > 0.0f) { bot->timerVelocidad -= dt; if (bot->timerVelocidad <= 0.0f) bot->velocidad = 120.0f; }
    }

    handleInput(dt);

    if (jugador->activo) jugador->update(dt);
    if (estadoActual == PVP && jugador2->activo) jugador2->update(dt);
    if (estadoActual == PVE && bot->activo) bot->update(dt);

    // Manejo de Victoria / Derrota
    if (!gameOver) {
        if (estadoActual == PVE) {
            if (!jugador->activo) { gameOver = true; ganador = 2;}
            else if (!bot->activo) { gameOver = true; ganador = 1; PlaySound(fxVictoria); } // Si matas al bot, ganas
        }
        else if (estadoActual == PVP && (!jugador->activo || !jugador2->activo)) {
            gameOver = true;
            if (!jugador->activo && !jugador2->activo) ganador = 0;
            else if (!jugador->activo) ganador = 2;
            else ganador = 1;
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
        DrawText("BOMBERMAN", width/2 - 120, height/4, 40, BLACK);
        Color colorPVE = (opcionMenu == 0) ? RED : DARKGRAY;
        Color colorPVP = (opcionMenu == 1) ? RED : DARKGRAY;
        Color colorInfo = (opcionMenu == 2) ? RED : DARKGRAY;

        DrawText((opcionMenu == 0 ? "-> 1 Jugador vs Bot" : "   1 Jugador vs Bot"), width/2 - 140, height/2, 20, colorPVE);
        DrawText((opcionMenu == 1 ? "-> Modo Versus" : "   Modo Versus"), width/2 - 140, height/2 + 40, 20, colorPVP);
        DrawText((opcionMenu == 2 ? "-> Info y Controles" : "   Info y Controles"), width/2 - 140, height/2 + 80, 20, colorInfo);
    }
    else if (estadoActual == PANTALLA_INFO) {
        DrawText("CONTROLES Y PODERES", width/2 - 180, 50, 30, BLACK);
        DrawText("Jugador 1: WASD para mover, ESPACIO para bomba", 50, 150, 20, DARKGRAY);
        DrawText("Jugador 2: Flechas para mover, ENTER para bomba", 50, 190, 20, DARKGRAY);

        DrawText("Power-Ups:", 50, 250, 20, RED);
        DrawText("- Fuego: Rango +1 bloque (Dura 15 segundos)", 50, 290, 20, DARKGRAY);
        DrawText("- Bomba: +1 bomba extra (Permanente)", 50, 330, 20, DARKGRAY);
        DrawText("- Velocidad: Velocidad +50 (Dura 15 segundos)", 50, 370, 20, DARKGRAY);

        DrawText("Presiona ESC o ENTER para volver", width/2 - 180, height - 50, 20, BLACK);
    }
    else if (estadoActual == PVP || estadoActual == PVE) {
        mapa->draw();

        auto p = powerups.cabeza;    while(p) { p->dato->draw(); p = p->siguiente; }
        auto b = bombas.cabeza;      while(b) { b->dato->draw(); b = b->siguiente; }
        auto e = explosiones.cabeza; while(e) { e->dato->draw(); e = e->siguiente; }

        if (jugador->activo) jugador->draw();
        if (estadoActual == PVP && jugador2->activo) jugador2->draw();
        if (estadoActual == PVE && bot->activo) bot->draw();
        // HUD Jugador 1
        DrawText("JUGADOR 1", 20, 10, 20, BLACK);
        int offsetX1 = 20;
        if (jugador->timerFuego > 0) { DrawRectangle(offsetX1, 35, 20, 20, RED); DrawText("F", offsetX1 + 5, 37, 16, WHITE); offsetX1 += 25; }
        if (jugador->timerVelocidad > 0) { DrawRectangle(offsetX1, 35, 20, 20, BLUE); DrawText("V", offsetX1 + 5, 37, 16, WHITE); }

        // HUD Jugador 2 / BOT
        if (estadoActual == PVP) {
            DrawText("JUGADOR 2", width - 130, 10, 20, BLACK);
            int offsetX2 = width - 40;
            if (jugador2->timerVelocidad > 0) { DrawRectangle(offsetX2, 35, 20, 20, BLUE); DrawText("V", offsetX2 + 5, 37, 16, WHITE); offsetX2 -= 25; }
            if (jugador2->timerFuego > 0) { DrawRectangle(offsetX2, 35, 20, 20, RED); DrawText("F", offsetX2 + 5, 37, 16, WHITE); }
        } else if (estadoActual == PVE) {
            const char* nombreBot = "Anti-Yarasca BOT";
            int anchoTexto = MeasureText(nombreBot, 20);
            int margenDerecho = 20;
            DrawText(nombreBot, width - anchoTexto - margenDerecho, 10, 20, RED);
            int offsetXBot = width - margenDerecho - 20;
            if (bot->timerVelocidad > 0) { DrawRectangle(offsetXBot, 35, 20, 20, BLUE); DrawText("V", offsetXBot + 5, 37, 16, WHITE); offsetXBot -= 25; }
            if (bot->timerFuego > 0) { DrawRectangle(offsetXBot, 35, 20, 20, RED); DrawText("F", offsetXBot + 5, 37, 16, WHITE); }
        }

        // Modo debug para ver las hitboxes
        if (debugMode && quadtree != nullptr) {
            quadtree->drawDebug();
            DrawRectangleLines(jugador->caja.centro.x - jugador->caja.medio, jugador->caja.centro.y - jugador->caja.medio, jugador->caja.medio * 2, jugador->caja.medio * 2, BLUE);
            if (estadoActual == PVP) DrawRectangleLines(jugador2->caja.centro.x - jugador2->caja.medio, jugador2->caja.centro.y - jugador2->caja.medio, jugador2->caja.medio * 2, jugador2->caja.medio * 2, RED);
            if (estadoActual == PVE) DrawRectangleLines(bot->caja.centro.x - bot->caja.medio, bot->caja.centro.y - bot->caja.medio, bot->caja.medio * 2, bot->caja.medio * 2, RED);
        }

        if (gameOver) {
            DrawRectangle(0, height/2 - 80, width, 180, Fade(BLACK, 0.8f));
            if (estadoActual == PVP) {
                if (ganador == 1) DrawText("¡JUGADOR 1 GANA!", width/2 - 160, height/2 - 50, 40, GREEN);
                else if (ganador == 2) DrawText("¡JUGADOR 2 GANA!", width/2 - 160, height/2 - 50, 40, BLUE);
                else DrawText("¡EMPATE!", width/2 - 90, height/2 - 50, 40, YELLOW);
            } else {
                if (ganador == 1) DrawText("¡VICTORIA!", width/2 - 100, height/2 - 50, 40, GREEN);
                else DrawText("DERROTA...", width/2 - 110, height/2 - 50, 40, RED);
            }
            DrawText("Presiona ENTER para volver al menu", width/2 - 190, height/2 + 20, 20, LIGHTGRAY);
        }
    }

    if (debugMode) DrawText("DEBUG MODE ON", 10, height - 30, 20, RED);
    DrawFPS(width - 90, height - 30);
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

void Game::pensarBot(float dt, float& dx, float& dy, bool& ponerBomba) {
    if (!bot->activo) return;
    ponerBomba = false;
    float vel = bot->velocidad * dt;
    int TILE = 40.0f;
    int cols = width / TILE;
    int rows = height / TILE;

    float bx = bot->caja.centro.x;
    float by = bot->caja.centro.y;
    int bCol = (int)(bx / TILE);
    int bRow = (int)(by / TILE);

    if(bCol < 0) bCol = 0; if(bCol >= cols) bCol = cols - 1;
    if(bRow < 0) bRow = 0; if(bRow >= rows) bRow = rows - 1;

    // Construccion del mapa mental del bot
    std::vector<std::vector<int>> grid(cols, std::vector<int>(rows, 0));

    auto m = mapa->muros.cabeza;
    while(m) {
        if(m->dato->activo) {
            int c = m->dato->caja.centro.x / TILE; int r = m->dato->caja.centro.y / TILE;
            if(c>=0 && c<cols && r>=0 && r<rows) grid[c][r] = (m->dato->tipo == TIPO_MURO_INDESTRUCTIBLE) ? 1 : 2;
        }
        m = m->siguiente;
    }

    auto p = powerups.cabeza;
    while(p) {
        if(p->dato->activo) {
            int c = p->dato->caja.centro.x / TILE; int r = p->dato->caja.centro.y / TILE;
            if(c>=0 && c<cols && r>=0 && r<rows) grid[c][r] = 5;
        }
        p = p->siguiente;
    }

    auto b = bombas.cabeza;
    while(b) {
        if(b->dato->activo) {
            int c = b->dato->caja.centro.x / TILE; int r = b->dato->caja.centro.y / TILE;
            if(c>=0 && c<cols && r>=0 && r<rows) {
                grid[c][r] = 3;
                int rango = b->dato->poderFuego;
                auto proyectar = [&](int dc, int dr) {
                    for(int i=1; i<=rango; i++) {
                        int nc = c + dc*i; int nr = r + dr*i;
                        if(nc<0 || nc>=cols || nr<0 || nr>=rows) break;
                        if(grid[nc][nr] == 1 || grid[nc][nr] == 2) break;
                        grid[nc][nr] = 4;
                    }
                };
                proyectar(0, -1); proyectar(0, 1); proyectar(-1, 0); proyectar(1, 0);
            }
        }
        b = b->siguiente;
    }

    auto e = explosiones.cabeza;
    while(e) {
        if(e->dato->activo) {
            int c = e->dato->caja.centro.x / TILE; int r = e->dato->caja.centro.y / TILE;
            if(c>=0 && c<cols && r>=0 && r<rows) grid[c][r] = 4;
        }
        e = e->siguiente;
    }

    if (grid[bCol][bRow] == 3) grid[bCol][bRow] = 4;
    auto buscarRuta = [&](std::function<bool(int, int)> esMeta, bool ignorarPeligro) -> std::pair<int, int> {
        std::queue<std::pair<int, int>> cola;
        std::vector<std::vector<std::pair<int, int>>> padre(cols, std::vector<std::pair<int, int>>(rows, {-1, -1}));
        std::vector<std::vector<bool>> visitado(cols, std::vector<bool>(rows, false));

        cola.push({bCol, bRow}); visitado[bCol][bRow] = true;
        int destC = -1, destR = -1;
        int dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}};

        while(!cola.empty()) {
            auto curr = cola.front(); cola.pop();
            int cc = curr.first; int cr = curr.second;

            if (esMeta(cc, cr)) { destC = cc; destR = cr; break; }

            for(int i=0; i<4; i++) {
                int nc = cc + dirs[i][0]; int nr = cr + dirs[i][1];
                if(nc>=0 && nc<cols && nr>=0 && nr<rows) {
                    bool puedePasar = ignorarPeligro ? (grid[nc][nr] == 0 || grid[nc][nr] == 4 || grid[nc][nr] == 5)
                                                     : (grid[nc][nr] == 0 || grid[nc][nr] == 5);
                    if(!visitado[nc][nr] && puedePasar) {
                        visitado[nc][nr] = true; padre[nc][nr] = {cc, cr}; cola.push({nc, nr});
                    }
                }
            }
        }

        if (destC == -1) return {-1, -1};
        if (destC == bCol && destR == bRow) return {bCol, bRow};

        int cC = destC, cR = destR;
        while(padre[cC][cR].first != bCol || padre[cC][cR].second != bRow) {
            auto p = padre[cC][cR]; cC = p.first; cR = p.second;
            // PARCHE ANTI-CRASHEO 2: Seguridad extrema por si se rompe el enlace
            if (cC == -1 || cR == -1) break;
        }
        return {cC, cR};
    };

    auto simularEscape = [&](int c, int r, int poder) -> bool {
        auto gridCopia = grid;
        gridCopia[c][r] = 3;
        auto proyectar = [&](int dc, int dr) {
            for(int i=1; i<=poder; i++) {
                int nc = c + dc*i; int nr = r + dr*i;
                if(nc<0 || nc>=cols || nr<0 || nr>=rows) break;
                if(gridCopia[nc][nr] == 1 || gridCopia[nc][nr] == 2) break;
                gridCopia[nc][nr] = 4;
            }
        };
        proyectar(0, -1); proyectar(0, 1); proyectar(-1, 0); proyectar(1, 0);
        gridCopia[c][r] = 4;

        std::queue<std::pair<int, int>> cola;
        std::vector<std::vector<bool>> visitado(cols, std::vector<bool>(rows, false));
        cola.push({c, r}); visitado[c][r] = true;
        int dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}};

        while(!cola.empty()) {
            auto curr = cola.front(); cola.pop();
            int cc = curr.first; int cr = curr.second;

            if (gridCopia[cc][cr] == 0 || gridCopia[cc][cr] == 5) return true;

            for(int i=0; i<4; i++) {
                int nc = cc + dirs[i][0]; int nr = cr + dirs[i][1];
                if(nc>=0 && nc<cols && nr>=0 && nr<rows) {
                    if(!visitado[nc][nr] && (gridCopia[nc][nr] == 0 || gridCopia[nc][nr] == 4 || gridCopia[nc][nr] == 5)) {
                        visitado[nc][nr] = true; cola.push({nc, nr});
                    }
                }
            }
        }
        return false;
    };

    // Maquina de Estados
    bool enPeligro = (grid[bCol][bRow] == 4);
    int sigC = bCol, sigR = bRow;
    bool esperando = false;

    if (enPeligro) {
        auto escape = buscarRuta([&](int c, int r) { return grid[c][r] == 0 || grid[c][r] == 5; }, true);
        if (escape.first != -1) { sigC = escape.first; sigR = escape.second; }
    }
    else {
        bool fuegoCerca = false;
        int dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
        for(int i=0; i<4; i++) {
            int nc = bCol + dirs[i][0]; int nr = bRow + dirs[i][1];
            if(nc>=0 && nc<cols && nr>=0 && nr<rows && grid[nc][nr] == 4) fuegoCerca = true;
        }

        if (fuegoCerca) esperando = true;
        else {
            int jCol = (int)(jugador->caja.centro.x / TILE);
            int jRow = (int)(jugador->caja.centro.y / TILE);

            if (bot->bombasActivas == 0) {
                bool atrapaJugador = false;
                bool rompeMuro = false;

                if (bCol == jCol && abs(bRow - jRow) <= bot->poderFuego) {
                    atrapaJugador = true;
                    int minY = std::min(bRow, jRow); int maxY = std::max(bRow, jRow);
                    for(int y = minY + 1; y < maxY; y++) if(grid[bCol][y] == 1 || grid[bCol][y] == 2) atrapaJugador = false;
                }
                else if (bRow == jRow && abs(bCol - jCol) <= bot->poderFuego) {
                    atrapaJugador = true;
                    int minX = std::min(bCol, jCol); int maxX = std::max(bCol, jCol);
                    for(int x = minX + 1; x < maxX; x++) if(grid[x][bRow] == 1 || grid[x][bRow] == 2) atrapaJugador = false;
                }

                for(int i=0; i<4; i++) {
                    int nc = bCol + dirs[i][0]; int nr = bRow + dirs[i][1];
                    if(nc>=0 && nc<cols && nr>=0 && nr<rows && grid[nc][nr] == 2) rompeMuro = true;
                }

                if (atrapaJugador || rompeMuro) {
                    if (simularEscape(bCol, bRow, bot->poderFuego)) {
                        ponerBomba = true;
                        auto escapeInmediato = buscarRuta([&](int c, int r) { return grid[c][r] == 0 || grid[c][r] == 5; }, true);
                        if (escapeInmediato.first != -1) { sigC = escapeInmediato.first; sigR = escapeInmediato.second; }
                    }
                }
            }

            if (!ponerBomba && !esperando) {
                auto irPowerUp = buscarRuta([&](int c, int r) { return grid[c][r] == 5; }, false);
                if (irPowerUp.first != -1) {
                    sigC = irPowerUp.first; sigR = irPowerUp.second;
                }
                else {
                    auto irJugador = buscarRuta([&](int c, int r) { return (abs(c - jCol) + abs(r - jRow)) <= 1; }, false);
                    if (irJugador.first != -1) {
                        sigC = irJugador.first; sigR = irJugador.second;
                    } else {
                        auto irMuro = buscarRuta([&](int c, int r) {
                            return (c>0 && grid[c-1][r]==2) || (c<cols-1 && grid[c+1][r]==2) ||
                                   (r>0 && grid[c][r-1]==2) || (r<rows-1 && grid[c][r+1]==2);
                        }, false);
                        if (irMuro.first != -1) { sigC = irMuro.first; sigR = irMuro.second; }
                    }
                }
            }
        }
    }

    // Movimiento Fisico
    if (esperando) {
        float centroX = bCol * TILE + TILE / 2.0f;
        float centroY = bRow * TILE + TILE / 2.0f;
        if (abs(bx - centroX) > 1.0f) dx = (centroX > bx) ? vel : -vel;
        if (abs(by - centroY) > 1.0f) dy = (centroY > by) ? vel : -vel;
    }
    else if (sigC != bCol || sigR != bRow) {
        float destX = sigC * TILE + TILE / 2.0f;
        float destY = sigR * TILE + TILE / 2.0f;

        if (sigC != bCol) {
            dx = (destX > bx) ? vel : -vel;
            bot->mirando = (dx > 0) ? DERECHA : IZQUIERDA;
            float centroY = bRow * TILE + TILE / 2.0f;
            if (abs(by - centroY) > 2.0f) dy = (centroY > by) ? vel : -vel;
        }
        else if (sigR != bRow) {
            dy = (destY > by) ? vel : -vel;
            bot->mirando = (dy > 0) ? ABAJO : ARRIBA;
            float centroX = bCol * TILE + TILE / 2.0f;
            if (abs(bx - centroX) > 2.0f) dx = (centroX > bx) ? vel : -vel;
        }
    }
}

void Game::moverYColisionar(Player* p, float dx, float dy) {
    if (!p->activo) return;

    if (dx != 0.0f) {
        Punto futuroX = p->caja.centro; futuroX.x += dx; AABB cajaX = {futuroX, p->caja.medio};
        Quadtree<Entity>::Nodo* resX = nullptr; quadtree->consultar(cajaX, &resX);
        bool chocaX = false; auto actualX = resX;
        while (actualX) {
            Entity* ent = actualX->datos;
            if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaX = true; }
            else if (ent->solido) chocaX = true;
            if (ent->tipo == TIPO_EXPLOSION) p->activo = false;
            if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(p, ent);
            auto aBorrar = actualX; actualX = actualX->siguiente; delete aBorrar;
        }
        if (!chocaX) p->caja.centro.x = futuroX.x;
    }

    if (dy != 0.0f) {
        Punto futuroY = p->caja.centro; futuroY.y += dy; AABB cajaY = {futuroY, p->caja.medio};
        Quadtree<Entity>::Nodo* resY = nullptr; quadtree->consultar(cajaY, &resY);
        bool chocaY = false; auto actualY = resY;
        while (actualY) {
            Entity* ent = actualY->datos;
            if (ent->tipo == TIPO_BOMBA) { if (!((Bomb*)ent)->recienColocada) chocaY = true; }
            else if (ent->solido) chocaY = true;
            if (ent->tipo == TIPO_EXPLOSION) p->activo = false;
            if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(p, ent);
            auto aBorrar = actualY; actualY = actualY->siguiente; delete aBorrar;
        }
        if (!chocaY) p->caja.centro.y = futuroY.y;
    }

    Quadtree<Entity>::Nodo* resImpacto = nullptr; quadtree->consultar(p->caja, &resImpacto); auto actImp = resImpacto;
    while (actImp) {
        Entity* entImp = actImp->datos;
        if (entImp->tipo == TIPO_EXPLOSION) p->activo = false;
        if (entImp->tipo == TIPO_POWERUP && entImp->activo) aplicarPowerUp(p, entImp);
        auto aBorrar = actImp; actImp = actImp->siguiente; delete aBorrar;
    }
}

void Game::run() {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(dt);
        render();
    }
}