#include "game.h"
#include <raylib.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

char const tetromino_templates[NUM_TETROMINO_TYPES][BLOCKS_SIZE] = {
    "XX  "
    " X  "
    " X  "
    "    ",

    " XX "
    " X  "
    " X  "
    "    ",

    "    "
    "XXX "
    " X  "
    "    ",

    "XX  "
    "XX  "
    "    "
    "    ",

    " X  "
    " X  "
    " X  "
    " X  ",

    " X  "
    "XX  "
    "X   "
    "    ",

    "X   "
    "XX  "
    " X  "
    "    "
};

size_t const tetromino_rotate_sizes[NUM_TETROMINO_TYPES] = {3, 3, 3, 2, 4, 3, 3};

Color const tetromino_colors[NUM_TETROMINO_TYPES] = {
    RED,
    YELLOW,
    GREEN,
    LIME,
    PURPLE,
    GOLD,
    SKYBLUE
};

TetrominoType random_tetromino_type(void) {
    return GetRandomValue(first_random_tetromino, last_random_tetromino);
}

Tetromino new_tetromino(TetrominoType type) { 
    Tetromino tetromino = {
        .x = cols / 2,
        .y = 2,
        .rotation = 0,
        .type = type,
    };

    strcpy(tetromino.blocks, tetromino_templates[type]);

    return tetromino;
}

GameState init_gamestate(void) {
    GameState game_state = {
        .current_piece = new_tetromino(random_tetromino_type()),
        .next_piece = random_tetromino_type(),
        .level = 1,
        .score = 0 
    };

    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            game_state.board[y][x] = NO_TETROMINO;
        }
    }

    return game_state;
}

void next_gamestate(GameState *const game_state) {
    // TODO: Game update
}

void display_game(GameState *const game_state) {
    // TODO: Display
}
