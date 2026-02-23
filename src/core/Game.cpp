#include "core/Game.h"
#include <stdlib.h>
#include <functional>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//Emscripten
static Game* instanciaGlobal = nullptr;

static void BucleWeb() {
    if (instanciaGlobal) {
        float dt = GetFrameTime();
        instanciaGlobal->update(dt);
        instanciaGlobal->render();
    }
}

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
    width = screenWidth;
    height = screenHeight;
    estadoActual = MENU_PRINCIPAL;
    opcionMenu = 0;
    debugMode = false;
    ganador = 0;
    gameOver = false;
    quadtree = nullptr;

    InitWindow(width, height, "Bomberman");
    InitAudioDevice();
// Thiago, elimine los sonidos xdddd
    fxExplosion = LoadSound("assets/sounds/explosion.wav");
    fxVictoria  = LoadSound("assets/sounds/victoria.wav");
    fxPickUp    = LoadSound("assets/sounds/powerup.wav");

    SetTargetFPS(60);

    texMenu      = LoadTexture("assets/textures/ui/Menu.png");
    texGameOver  = LoadTexture("assets/textures/ui/GameOver.png");
    texBomberman = LoadTexture("assets/textures/characters/characters.png");
    texTiles     = LoadTexture("assets/textures/items/tiles.png");
    texArena     = LoadTexture("assets/textures/tiles/Arena.png");

    SetTextureFilter(texMenu,      TEXTURE_FILTER_POINT);
    SetTextureFilter(texGameOver,  TEXTURE_FILTER_POINT);
    SetTextureFilter(texBomberman, TEXTURE_FILTER_POINT);
    SetTextureFilter(texTiles,     TEXTURE_FILTER_POINT);
    SetTextureFilter(texArena,     TEXTURE_FILTER_POINT);

    mapa = new Map();
    mapa->setTileTexture(texTiles);
    mapa->cargarMapa(1);

    jugador = new Player(60.0f, 60.0f);
    jugador->setTexture(texBomberman, 1);   // blanco → offset=0

    jugador2 = new Player(width - 60.0f, height - 60.0f);
    jugador2->setTexture(texBomberman, 2);  // negro  → offset=103
    jugador2->activo = false;

    bot = new Player(width - 60.0f, height - 60.0f);
    bot->setTexture(texBomberman, 2);       // blanco también
    bot->activo = false;
    bot->velocidad = 120.0f;
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

    UnloadTexture(texMenu);
    UnloadTexture(texGameOver);
    UnloadTexture(texBomberman);
    UnloadTexture(texTiles);
    UnloadTexture(texArena);

    CloseWindow();
}

Bomb* Game::crearBomba(float cx, float cy, int poder, Player* prop, int playerNumber) {
    Bomb* b = new Bomb(cx, cy, poder, prop);
    b->initAnimations(texTiles, playerNumber);
    return b;
}

void Game::handleInput(float dt) {
    float dx1 = 0.0f, dy1 = 0.0f;
    float dx2 = 0.0f, dy2 = 0.0f;

	//P1
    if (jugador->activo && jugador->isAlive) {
        float vel = jugador->velocidad * dt;
        bool mX = false, mY = false;

        if (IsKeyDown(KEY_A)) { dx1 -= vel; jugador->updateAnimX(false); mX = true; }
        if (IsKeyDown(KEY_D)) { dx1 += vel; jugador->updateAnimX(true);  mX = true; }
        if (!mX) jugador->stopAnimX();

        if (IsKeyDown(KEY_W)) { dy1 -= vel; jugador->updateAnimY(false); mY = true; }
        if (IsKeyDown(KEY_S)) { dy1 += vel; jugador->updateAnimY(true);  mY = true; }
        if (!mY) jugador->stopAnimY();

        if (IsKeyPressed(KEY_SPACE) && jugador->bombasActivas < jugador->maxBombas) {
            const float TILE = 40.0f;
            float cx = ((int)(jugador->caja.centro.x / TILE)) * TILE + TILE / 2.0f;
            float cy = ((int)(jugador->caja.centro.y / TILE)) * TILE + TILE / 2.0f;
            bombas.insertar(crearBomba(cx, cy, jugador->poderFuego, jugador, 1));
            jugador->bombasActivas++;
        }
    }

    // p2
    if (estadoActual == PVP && jugador2->activo && jugador2->isAlive) {
        float vel2 = jugador2->velocidad * dt;
        bool mX2 = false, mY2 = false;

        if (IsKeyDown(KEY_LEFT))  { dx2 -= vel2; jugador2->updateAnimX(false); mX2 = true; }
        if (IsKeyDown(KEY_RIGHT)) { dx2 += vel2; jugador2->updateAnimX(true);  mX2 = true; }
        if (!mX2) jugador2->stopAnimX();

        if (IsKeyDown(KEY_UP))   { dy2 -= vel2; jugador2->updateAnimY(false); mY2 = true; }
        if (IsKeyDown(KEY_DOWN)) { dy2 += vel2; jugador2->updateAnimY(true);  mY2 = true; }
        if (!mY2) jugador2->stopAnimY();

        if (IsKeyPressed(KEY_ENTER) && jugador2->bombasActivas < jugador2->maxBombas) {
            const float TILE = 40.0f;
            float cx = ((int)(jugador2->caja.centro.x / TILE)) * TILE + TILE / 2.0f;
            float cy = ((int)(jugador2->caja.centro.y / TILE)) * TILE + TILE / 2.0f;
            bombas.insertar(crearBomba(cx, cy, jugador2->poderFuego, jugador2, 2));
            jugador2->bombasActivas++;
        }
    }

    // quadtree
    if (quadtree) delete quadtree;
    quadtree = new Quadtree<Entity>(AABB{{width/2.0f, height/2.0f}, width/2.0f}, 4);
    auto m = mapa->muros.cabeza;
    while(m) { if(m->dato->activo) quadtree->insertar(m->dato); m = m->siguiente; }
    auto b = bombas.cabeza;
    while(b) { if(b->dato->activo) quadtree->insertar(b->dato); b = b->siguiente; }
    auto e = explosiones.cabeza;
    while(e) { if(e->dato->activo) quadtree->insertar(e->dato); e = e->siguiente; }
    auto p = powerups.cabeza;
    while(p) { if(p->dato->activo) quadtree->insertar(p->dato); p = p->siguiente; }

    moverYColisionar(jugador, dx1, dy1);
    if (estadoActual == PVP) moverYColisionar(jugador2, dx2, dy2);

    //bot
    if (estadoActual == PVE && bot->activo) {
        float dxBot = 0.0f, dyBot = 0.0f;
        bool botPoneBomba = false;
        pensarBot(dt, dxBot, dyBot, botPoneBomba);
        moverYColisionar(bot, dxBot, dyBot);

        if (botPoneBomba && bot->bombasActivas < bot->maxBombas) {
            const float TILE = 40.0f;
            float cx = ((int)(bot->caja.centro.x / TILE)) * TILE + TILE / 2.0f;
            float cy = ((int)(bot->caja.centro.y / TILE)) * TILE + TILE / 2.0f;
            bombas.insertar(crearBomba(cx, cy, bot->poderFuego, bot, 1));
            bot->bombasActivas++;
        }
    }
}

void Game::generarExplosion(Bomb* bomba) {
    const float TILE = 40.0f;
    float cx = bomba->caja.centro.x;
    float cy = bomba->caja.centro.y;
    int poder = bomba->poderFuego;

    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    int* radii[4] = { &bomba->rRadius, &bomba->lRadius,
                      &bomba->dRadius, &bomba->uRadius };

    for (int d = 0; d < 4; d++) {
        *radii[d] = poder;
        for (int i = 1; i <= poder; i++) {
            float tx = cx + dirs[d][0] * TILE * i;
            float ty = cy + dirs[d][1] * TILE * i;
            AABB celda = {{tx, ty}, 2.0f};
            Quadtree<Entity>::Nodo* res = nullptr;
            quadtree->consultar(celda, &res);
            bool parar = false;
            auto cur = res;
            while (cur) {
                Entity* ent = cur->datos;
                if (ent->tipo == TIPO_MURO_INDESTRUCTIBLE) {
                    *radii[d] = i - 1;
                    parar = true;
                } else if (ent->tipo == TIPO_MURO_DESTRUCTIBLE) {
                    WallEntity* w = static_cast<WallEntity*>(ent);
                    w->startDestroy();
                    *radii[d] = i;
                    parar = true;
                    if (rand() % 100 < 30) {
                        int dado = rand() % 3;
                        TipoPowerUp t = (dado==0) ? PWR_FUEGO
                                      : (dado==1) ? PWR_BOMBA
                                      :             PWR_VELOCIDAD;
                        powerups.insertar(new PowerUp(tx, ty, t));
                    }
                } else if (ent->tipo == TIPO_JUGADOR) {
                    ((Player*)ent)->die();
                } else if (ent->tipo == TIPO_BOMBA) {
                    ((Bomb*)ent)->tiempoRestante = 0.0f;
                }
                auto del = cur; cur = cur->siguiente; delete del;
            }
            if (parar) break;
        }
    }

    bomba->explode();
}

void Game::update(float dt) {
    if (IsKeyPressed(KEY_F3)) debugMode = !debugMode;

    if (estadoActual == MENU_PRINCIPAL) {
        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) { opcionMenu--; if (opcionMenu < 0) opcionMenu = 2; }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { opcionMenu++; if (opcionMenu > 2) opcionMenu = 0; }
        if (IsKeyPressed(KEY_ENTER)) {
            if      (opcionMenu == 0) { estadoActual = PVE; bot->activo = true; jugador2->activo = false; }
            else if (opcionMenu == 1) { estadoActual = PVP; jugador2->activo = true; bot->activo = false; }
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
            gameOver = false; ganador = 0;

            jugador->activo = true;
            jugador->isAlive = true;
            jugador->caja.centro = {60.0f, 60.0f};
            jugador->maxBombas = 1; jugador->bombasActivas = 0;
            jugador->poderFuego = 1; jugador->velocidad = 150.0f;
            jugador->timerFuego = 0; jugador->timerVelocidad = 0;

            jugador2->isAlive = true;
            jugador2->caja.centro = {(float)width-60.0f, (float)height-60.0f};
            jugador2->maxBombas = 1; jugador2->bombasActivas = 0;
            jugador2->poderFuego = 1; jugador2->velocidad = 150.0f;
            jugador2->timerFuego = 0; jugador2->timerVelocidad = 0;

            bot->isAlive = true;
            bot->caja.centro = {(float)width-60.0f, (float)height-60.0f};
            bot->maxBombas = 1; bot->bombasActivas = 0;
            bot->poderFuego = 1; bot->velocidad = 120.0f;
            bot->timerFuego = 0; bot->timerVelocidad = 0;

            bombas.limpiar(); explosiones.limpiar(); powerups.limpiar();
            mapa->cargarMapa(1);
        }
        return;
    }

    auto tickTimer = [](float& timer, float& stat, float reset, float dt) {
        if (timer > 0) { timer -= dt; if (timer <= 0) stat = reset; }
    };
    tickTimer(jugador->timerFuego,     (float&)jugador->poderFuego, 1.0f, dt);
    tickTimer(jugador->timerVelocidad, jugador->velocidad,         150.0f, dt);
    if (estadoActual == PVP && jugador2->activo) {
        tickTimer(jugador2->timerFuego,     (float&)jugador2->poderFuego, 1.0f, dt);
        tickTimer(jugador2->timerVelocidad, jugador2->velocidad,         150.0f, dt);
    }
    if (estadoActual == PVE && bot->activo) {
        tickTimer(bot->timerFuego,     (float&)bot->poderFuego, 1.0f, dt);
        tickTimer(bot->timerVelocidad, bot->velocidad,         120.0f, dt);
    }

    handleInput(dt);

    if (jugador->activo)                          jugador->update(dt);
    if (estadoActual == PVP && jugador2->activo)  jugador2->update(dt);
    if (estadoActual == PVE && bot->activo)       bot->update(dt);

    if (!gameOver) {
        if (estadoActual == PVE) {
            if (!jugador->activo)  { gameOver = true; ganador = 2; }
            else if (!bot->activo) { gameOver = true; ganador = 1; PlaySound(fxVictoria); }
        } else if (estadoActual == PVP && (!jugador->activo || !jugador2->activo)) {
            gameOver = true;
            if (!jugador->activo && !jugador2->activo) ganador = 0;
            else if (!jugador->activo) ganador = 2;
            else ganador = 1;
            PlaySound(fxVictoria);
        }
    }

    auto b = bombas.cabeza;
    while (b) {
        Bomb* bomba = (Bomb*)b->dato;
        bomba->update(dt);
        if (!bomba->isExploding && bomba->tiempoRestante <= 0.0f) {
            PlaySound(fxExplosion);
            generarExplosion(bomba);
            if (bomba->propietario) bomba->propietario->bombasActivas--;
        }
        b = b->siguiente;
    }

    mapa->update(dt);
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
        ClearBackground({0, 89, 255, 255});
        Rectangle source = {0, 0, (float)texMenu.width, (float)texMenu.height / 3.0f};
        float scale = (float)width / texMenu.width;
        DrawTexturePro(texMenu, source, {0,0,(float)width, source.height*scale}, {0,0}, 0, WHITE);

        Color cPVE  = (opcionMenu==0) ? YELLOW : WHITE;
        Color cPVP  = (opcionMenu==1) ? YELLOW : WHITE;
        Color cInfo = (opcionMenu==2) ? YELLOW : WHITE;
        float sy = height / 2.0f + 50.0f;
        DrawText("1 Jugador vs Bot", width/2-100, (int)sy,      25, cPVE);
        DrawText("Modo Versus",      width/2-100, (int)sy+40,   25, cPVP);
        DrawText("Info y Controles", width/2-100, (int)sy+80,   25, cInfo);
        int cy = (opcionMenu==0)?(int)sy:(opcionMenu==1)?(int)sy+40:(int)sy+80;
        DrawText("->", width/2-140, cy, 25, YELLOW);
    }

    else if (estadoActual == PANTALLA_INFO) {
        ClearBackground(DARKGRAY);
        DrawText("CONTROLES Y PODERES",                            width/2-180,50, 30,WHITE);
        DrawText("Jugador 1: WASD para mover, ESPACIO para bomba", 50,150,20,LIGHTGRAY);
        DrawText("Jugador 2: Flechas para mover, ENTER para bomba",50,190,20,LIGHTGRAY);
        DrawText("Power-Ups:",                                      50,250,20,RED);
        DrawText("- Fuego: Rango +1 bloque",    50,290,20,LIGHTGRAY);
        DrawText("- Bomba: +1 bomba extra (Permanente)",            50,330,20,LIGHTGRAY);
        DrawText("- Velocidad: Velocidad +50",  50,370,20,LIGHTGRAY);
        DrawText("Presiona ESC o ENTER para volver",width/2-180,height-50,20,WHITE);
    }

    // Juego
    else if (estadoActual == PVP || estadoActual == PVE) {

        DrawTexturePro(texArena,
            {0,0,(float)texArena.width,(float)texArena.height},
            {0,0,(float)width,(float)height},
            {0,0}, 0, WHITE);

        mapa->draw();

        auto p = powerups.cabeza;
        while (p) {
            if (p->dato->activo) {
                PowerUp* pwr = (PowerUp*)p->dato;
                Rectangle srcPwr;
                if      (pwr->tipoPoder == PWR_BOMBA)     srcPwr = {372.0f,  0.0f, 16.0f, 16.0f};
                else if (pwr->tipoPoder == PWR_FUEGO)     srcPwr = {388.0f,  0.0f, 16.0f, 16.0f};
                else /* PWR_VELOCIDAD */                  srcPwr = {388.0f, 16.0f, 16.0f, 16.0f};
                Rectangle dst = {
                    pwr->caja.centro.x - 20.0f,
                    pwr->caja.centro.y - 20.0f,
                    40.0f, 40.0f
                };
                DrawTexturePro(texTiles, srcPwr, dst, {0,0}, 0, WHITE);
            }
            p = p->siguiente;
        }

        auto b = bombas.cabeza;
        while (b) {
            if (b->dato->activo) ((Bomb*)b->dato)->draw();
            b = b->siguiente;
        }

        if (jugador->activo)                         jugador->draw();
        if (estadoActual==PVP && jugador2->activo)   jugador2->draw();
        if (estadoActual==PVE && bot->activo)        bot->draw();

        DrawText("JUGADOR 1", 20, 10, 20, BLACK);
        int ox1 = 20;
        if (jugador->timerFuego     > 0) { DrawRectangle(ox1,35,20,20,RED);  DrawText("F",ox1+5,37,16,WHITE); ox1+=25; }
        if (jugador->timerVelocidad > 0) { DrawRectangle(ox1,35,20,20,BLUE); DrawText("V",ox1+5,37,16,WHITE); }

        if (estadoActual == PVP) {
            DrawText("JUGADOR 2", width-130, 10, 20, BLACK);
            int ox2 = width-40;
            if (jugador2->timerVelocidad>0){DrawRectangle(ox2,35,20,20,BLUE);DrawText("V",ox2+5,37,16,WHITE);ox2-=25;}
            if (jugador2->timerFuego    >0){DrawRectangle(ox2,35,20,20,RED); DrawText("F",ox2+5,37,16,WHITE);}
        } else {
            const char* nb = "Anti-Yarasca BOT";
            DrawText(nb, width-MeasureText(nb,20)-20, 10, 20, RED);
            int oxB = width-40;
            if (bot->timerVelocidad>0){DrawRectangle(oxB,35,20,20,BLUE);DrawText("V",oxB+5,37,16,WHITE);oxB-=25;}
            if (bot->timerFuego    >0){DrawRectangle(oxB,35,20,20,RED); DrawText("F",oxB+5,37,16,WHITE);}
        }

        if (debugMode && quadtree) {
            quadtree->drawDebug();
            auto box = [&](Player* pl, Color c) {
                DrawRectangleLines(
                    (int)(pl->caja.centro.x - pl->caja.medio),
                    (int)(pl->caja.centro.y - pl->caja.medio),
                    (int)(pl->caja.medio*2), (int)(pl->caja.medio*2), c);
            };
            box(jugador, BLUE);
            if (estadoActual==PVP) box(jugador2, RED);
            if (estadoActual==PVE) box(bot, RED);
        }

        if (gameOver) {
            DrawRectangle(0,0,width,height,Fade(BLACK,0.7f));
            DrawTexturePro(texGameOver,
                {0,0,(float)texGameOver.width/2.0f,(float)texGameOver.height*0.33f},
                {width/2.0f-180.0f, height/2.0f-150.0f, 360.0f, 180.0f},
                {0,0}, 0, WHITE);
            int ty = height/2+50;
            if (estadoActual == PVP) {
                if (ganador==1)      DrawText("¡JUGADOR 1 GANA!",width/2-120,ty,25,GREEN);
                else if (ganador==2) DrawText("¡JUGADOR 2 GANA!",width/2-120,ty,25,BLUE);
                else                 DrawText("¡EMPATE!",         width/2-50, ty,25,YELLOW);
            } else {
                if (ganador==1) DrawText("¡VICTORIA!",width/2-70,ty,25,GREEN);
                else            DrawText("DERROTA...", width/2-70,ty,25,RED);
            }
            DrawText("Presiona ENTER para volver",width/2-140,ty+50,20,LIGHTGRAY);
        }
    }

    if (debugMode) DrawText("DEBUG MODE ON",10,height-30,20,RED);
    DrawFPS(width-90, height-30);
    EndDrawing();
}

void Game::aplicarPowerUp(Player* jug, Entity* ent) {
    ent->activo = false;
    PowerUp* pwr = (PowerUp*)ent;
    PlaySound(fxPickUp);
    if      (pwr->tipoPoder == PWR_BOMBA)     { jug->maxBombas++; }
    else if (pwr->tipoPoder == PWR_FUEGO)     { jug->poderFuego++;    jug->timerFuego     = 6000.0f; }
    else if (pwr->tipoPoder == PWR_VELOCIDAD) { jug->velocidad+=15.0f; jug->timerVelocidad = 6000.0f; }
}

void Game::moverYColisionar(Player* p, float dx, float dy) {
    if (!p->activo || !p->isAlive) return;

    auto bCheck = bombas.cabeza;
    while (bCheck) {
        Bomb* bomb = (Bomb*)bCheck->dato;
        if (bomb->activo && bomb->isExploding) {
            float TILE = 40.0f;
            float bx = bomb->caja.centro.x;
            float by = bomb->caja.centro.y;
            float px = p->caja.centro.x;
            float py = p->caja.centro.y;
            float margen = 15.0f;

            if (std::abs(px - bx) < margen) {
                float fuegoArriba = by - (bomb->uRadius * TILE) - (TILE/2.0f);
                float fuegoAbajo = by + (bomb->dRadius * TILE) + (TILE/2.0f);
                if (py >= fuegoArriba && py <= fuegoAbajo) {
                    p->die();
                }
            }
            else if (std::abs(py - by) < margen) {
                float fuegoIzquierda = bx - (bomb->lRadius * TILE) - (TILE/2.0f);
                float fuegoDerecha = bx + (bomb->rRadius * TILE) + (TILE/2.0f);
                if (px >= fuegoIzquierda && px <= fuegoDerecha) {
                    p->die();
                }
            }
        }
        bCheck = bCheck->siguiente;
    }

    if (!p->isAlive) return;

    auto procesarColisiones = [&](Quadtree<Entity>::Nodo* res, bool& choca) {
        auto cur = res;
        while (cur) {
            Entity* ent = cur->datos;
            if (ent->tipo == TIPO_BOMBA) {
                Bomb* laBomba = (Bomb*)ent;
                if (!laBomba->isExploding && !laBomba->recienColocada) {
                    choca = true;
                }
            } else if (ent->solido) {
                choca = true;
            }
            if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(p, ent);
            auto del = cur; cur = cur->siguiente; delete del;
        }
    };

    if (dx != 0.0f) {
        Punto futX = p->caja.centro; futX.x += dx;
        AABB cajaX = {futX, p->caja.medio};
        Quadtree<Entity>::Nodo* resX = nullptr;
        quadtree->consultar(cajaX, &resX);
        bool chocaX = false;
        procesarColisiones(resX, chocaX);
        if (!chocaX) p->caja.centro.x = futX.x;
    }

    if (dy != 0.0f) {
        Punto futY = p->caja.centro; futY.y += dy;
        AABB cajaY = {futY, p->caja.medio};
        Quadtree<Entity>::Nodo* resY = nullptr;
        quadtree->consultar(cajaY, &resY);
        bool chocaY = false;
        procesarColisiones(resY, chocaY);
        if (!chocaY) p->caja.centro.y = futY.y;
    }

    Quadtree<Entity>::Nodo* resF = nullptr;
    quadtree->consultar(p->caja, &resF);
    auto cur = resF;
    while (cur) {
        Entity* ent = cur->datos;
        if (ent->tipo == TIPO_POWERUP && ent->activo) aplicarPowerUp(p, ent);
        auto del = cur; cur = cur->siguiente; delete del;
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
                int rango = ((Bomb*)b->dato)->poderFuego; // <-- SOLUCIÓN
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

    if (esperando) {
        float centroX = bCol * TILE + TILE / 2.0f;
        float centroY = bRow * TILE + TILE / 2.0f;
        bot->stopAnimX();
        bot->stopAnimY();
        if (abs(bx - centroX) > 1.0f) {
            dx = (centroX > bx) ? vel : -vel;
            bot->updateAnimX(dx > 0);
        }
        if (abs(by - centroY) > 1.0f) {
            dy = (centroY > by) ? vel : -vel;
            bot->updateAnimY(dy > 0);
        }
    }
    else if (sigC != bCol || sigR != bRow) {
        float destX = sigC * TILE + TILE / 2.0f;
        float destY = sigR * TILE + TILE / 2.0f;

        if (sigC != bCol) {
            dx = (destX > bx) ? vel : -vel;
            bot->updateAnimX(dx > 0);
            float centroY = bRow * TILE + TILE / 2.0f;
            if (abs(by - centroY) > 2.0f) dy = (centroY > by) ? vel : -vel;
        }
        else if (sigR != bRow) {
            dy = (destY > by) ? vel : -vel;
            bot->updateAnimY(dy > 0);
            float centroX = bCol * TILE + TILE / 2.0f;
            if (abs(bx - centroX) > 2.0f) dx = (centroX > bx) ? vel : -vel;
        }
    } else {
        bot->stopAnimX();
        bot->stopAnimY();
    }
}


void Game::run() {
#if defined(PLATFORM_WEB)
    instanciaGlobal = this;
    emscripten_set_main_loop(BucleWeb, 0, 1);
#else
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(dt);
        render();
    }
#endif
}