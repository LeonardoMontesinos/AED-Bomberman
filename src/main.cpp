#include "core/Game.h"

int main() {
    // Inicializar el juego con resolución 800x600
    Game game(800, 600);

    // Iniciar el Game Loop
    game.run();

    return 0;
}