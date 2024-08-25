#include "game.h"
#include "config.h"
#include "load.h"
#include <stdlib.h>
#include <raylib.h>
#include <dlfcn.h>

int main(void) {
    void *libgame = dlopen_safe(libgame_path, RTLD_NOW);
    init_gamestate_t init_gamestate = dlsym_safe(libgame, "init_gamestate");
    next_gamestate_t next_gamestate = dlsym_safe(libgame, "next_gamestate");
    display_game_t   display_game   = dlsym_safe(libgame, "display_game");
    
    InitWindow(init_width, init_height, "Game!");
    SetTargetFPS(fps);

    GameState game_state = init_gamestate();
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            dlclose(libgame);
            libgame = dlopen_safe(libgame_path, RTLD_NOW);
        }
        next_gamestate(&game_state);
        display_game(&game_state);
    }
    CloseWindow();
    dlclose(libgame);

    return EXIT_SUCCESS;
}
