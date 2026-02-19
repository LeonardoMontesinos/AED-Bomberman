#include "entities/PowerUp.h"

PowerUp::PowerUp(float x, float y, TipoPowerUp poder) : Entity(x, y, 16.0f, YELLOW, false, TIPO_POWERUP) {
    tipoPoder = poder;
}

void PowerUp::update(float dt) {}

void PowerUp::draw() {
    if (activo) {
        DrawRectangle((int)(caja.centro.x - caja.medio),
                      (int)(caja.centro.y - caja.medio),
                      (int)(caja.medio * 2), (int)(caja.medio * 2), color);
        int posX = (int)caja.centro.x - 6;
        int posY = (int)caja.centro.y - 10;

        if (tipoPoder == PWR_BOMBA) {
            DrawText("B", posX, posY, 20, BLACK);
        } else if (tipoPoder == PWR_FUEGO) {
            DrawText("F", posX, posY, 20, RED);
        } else if (tipoPoder == PWR_VELOCIDAD) {
            DrawText("V", posX, posY, 20, BLUE);
        }
    }
}