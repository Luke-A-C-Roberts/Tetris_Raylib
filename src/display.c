#include "game.h"
#include <raylib.h>
#include <stdio.h>

// Display Functions ////////////////////////////////////////////////////////// 
static Color const tetromino_colors[NUM_TETROMINO_TYPES + 1]
    = {RED, YELLOW, GREEN, LIME, PURPLE, GOLD, SKYBLUE, GRAY};
  
static inline void _disp_current_tetromino(
    Tetromino const*const restrict tetromino
) {
    Color const color = tetromino_colors[tetromino->type]; 
    size_t const x_offset = tetromino->x;    
    size_t const y_offset = tetromino->y;

    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        size_t const x_absolute = tetromino->positions[i][X_AXIS] + x_offset;
        size_t const y_absolute = tetromino->positions[i][Y_AXIS] + y_offset;
        DrawRectangle(
            x_absolute * BLOCK_SCALE + X_OFFSET,
            y_absolute * BLOCK_SCALE + Y_OFFSET,
            BLOCK_SCALE,
            BLOCK_SCALE,
            color
        );
    }
}

static inline void _disp_boarders(void) {
    size_t const cols_mul = COLS * BLOCK_SCALE + X_OFFSET;
    size_t const rows_mul = ROWS * BLOCK_SCALE + Y_OFFSET; 
    DrawLine(X_OFFSET, Y_OFFSET, cols_mul, Y_OFFSET, BLACK);
    DrawLine(X_OFFSET, Y_OFFSET, X_OFFSET, rows_mul, BLACK);
    DrawLine(cols_mul, Y_OFFSET, cols_mul, rows_mul, BLACK);
    DrawLine(X_OFFSET, rows_mul, cols_mul, rows_mul, BLACK);  
}

static inline void _disp_blocks(TetrominoType const board[ROWS][COLS]) {
    for (size_t y = 0; y < ROWS; ++y) {
        for (size_t x = 0; x < COLS; ++x) DrawRectangle(
            x * BLOCK_SCALE + X_OFFSET,
            y * BLOCK_SCALE + Y_OFFSET,
            BLOCK_SCALE,
            BLOCK_SCALE,
            tetromino_colors[board[y][x]]
        );
    }
} 

static inline void _disp_info(GameState const*const gamestate) {
    static char temp_str[20];
    sprintf(temp_str, "%zu", gamestate->level);
    DrawText(
        temp_str,
        INFO_X_OFFSET,
        INFO_Y_OFFSET,
        INFO_FONT_SIZE,
        BLACK
    );

    sprintf(temp_str, "%zu", gamestate->score);
    DrawText(
        temp_str,
        INFO_X_OFFSET + INFO_NEXT_ITEM_X * 1,
        INFO_Y_OFFSET,
        INFO_FONT_SIZE,
        BLACK
    ); 
}

// Display Exposed ////////////////////////////////////////////////////////////
extern void display_game(GameState const*const game_state) {
    BeginDrawing();

    ClearBackground(GRAY);
    _disp_blocks(game_state->board);
    _disp_boarders();
    _disp_current_tetromino(&game_state->current_tetromino);
    _disp_info(game_state);
    
    EndDrawing();
}

