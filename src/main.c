#include "game.h"
#include "config.h"
#include "load.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <dlfcn.h>

int main(void) {

    // Ths ptr opens a shared object file or crashes if it can't find it
    void *libgame = dlopen_safe(libgame_path, RTLD_NOW);

    // Macro that loads functions safely (see load.h)
    LOAD_FUNC(libgame, init_gamestate);
    LOAD_FUNC(libgame, init_display_config);
    LOAD_FUNC(libgame, next_gamestate);
    LOAD_FUNC(libgame, display_game);

    // LOAD_FUNC(libgame, next_selection_screen_state);
    // LOAD_FUNC(libgame, disp_selection_screen);
    
    // Initialises raylib state to configure window
    InitWindow(INIT_WIDTH, INIT_HEIGHT, "Game!");
    SetTargetFPS(FPS);

    // Gamestate object holds all game objects, and gameloop updates it each cycle
    GameState game_state = init_gamestate(INIT_LEVEL);
    DisplayConfig display_config =
        init_display_config(game_state.display_mode);

    while (!WindowShouldClose()) {
        double const time = GetTime();
        if (IsKeyPressed(KEY_R)) {
            printf("============== Hot Reload =============\n\n");

            // close previous shared object link and open a fresh one
            dlclose(libgame);
            libgame = dlopen_safe(libgame_path, RTLD_NOW); 

            // reload functions
            RELOAD_FUNC(libgame, init_gamestate);
            RELOAD_FUNC(libgame, next_gamestate);
            RELOAD_FUNC(libgame, display_game);
            RELOAD_FUNC(libgame, init_display_config);
            // RELOAD_FUNC(libgame, next_selection_screen_state);
            // RELOAD_FUNC(libgame, disp_selection_screen);

            // DEBUG: refresh game state (optional)
            game_state = init_gamestate(INIT_LEVEL);
            display_config = init_display_config(game_state.display_mode);
        } 

        // TODO: add level selection screen
        /*
        size_t init_level = 0;
        if (game_state.is_on_selection_screen) {
            init_level = next_selection_screen_state(&game_state);
            disp_selection_screen(&game_state);
            continue;
        }
        */
        
        bool is_game_over = next_gamestate(&game_state); // Update game state
        display_game(&game_state, &display_config);   // Take gamestate and render it

        if (is_game_over) {
            // TODO: add gameover screen
            game_state = init_gamestate(INIT_LEVEL);
        }

        if (IsKeyPressed(KEY_P)) {
            printf("============== DEBUG INFO =============\n\n");
            print_game_state(&game_state);
        }

        double const time_delta = GetTime() - time;
        WaitTime(1. / GetFPS() - time_delta);
    }
    CloseWindow();
    dlclose(libgame);

    return EXIT_SUCCESS;
}
