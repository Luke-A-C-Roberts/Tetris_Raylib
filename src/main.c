#include "game.h"
#include "config.h"
#include "load.h"
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <dlfcn.h>

int main(void) {

    // Ths ptr opens a shared object file or crashes if it can't find it
    void *libgame = dlopen_safe(libgame_path, RTLD_NOW);

    // Macro that loads functions safely (see load.h)
    LOAD_FUNC(init_gamestate);
    LOAD_FUNC(next_gamestate);
    LOAD_FUNC(display_game);
    
    // Initialises raylib state to configure window
    InitWindow(INIT_WIDTH, INIT_HEIGHT, "Game!");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(FPS);

    // Gamestate object holds all game objects, and gameloop updates it each cycle
    GameState game_state = init_gamestate();
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            printf("============== Hot Reload =============\n\n");

            // close previous shared object link and open a fresh one
            dlclose(libgame);
            libgame = dlopen_safe(libgame_path, RTLD_NOW); 

            // reload functions
            RELOAD_FUNC(init_gamestate);
            RELOAD_FUNC(next_gamestate);
            RELOAD_FUNC(display_game);

            // DEBUG: refresh game state (optional)
            game_state = init_gamestate();
        }

        next_gamestate(&game_state); // Update game state
        display_game(&game_state);   // Take gamestate and render it
    }
    CloseWindow();
    dlclose(libgame);

    return EXIT_SUCCESS;
}
