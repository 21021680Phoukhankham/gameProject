#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include "Game.h"

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init()) {
        std::cout << "Không thể khởi tạo game!" << std::endl;
        return 1;
    }

    if (!game.loadMedia()) {
        std::cout << "Không thể tải tài nguyên game!" << std::endl;
        return 1;
    }

    // Vòng lặp game
    while (game.isRunning()) {
        game.handleEvents();
        game.update();
        game.render();
        
        // Giới hạn FPS (60 FPS)
        SDL_Delay(1000/60);
    }

    return 0;
}