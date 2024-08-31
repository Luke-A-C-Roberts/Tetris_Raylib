#include "game.h"
#include "config.h"
#include <raylib.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Data ///////////////////////////////////////////////////////////////////////
char const tetromino_templates[NUM_TETROMINO_TYPES][BLOCKS_SIZE] = {
    "XX  "
    " X  "
    " X  "
    "    ", "XX  "
            "X   "
            "X   "
            "    ", "    "
                    "XXX "
                    " X  "
                    "    ", "XX  "
                            "XX  "
                            "    "
                            "    ", " X  "
                                    " X  "
                                    " X  "
                                    " X  ", " X  "
                                            "XX  "
                                            "X   "
                                            "    ", "X   "
                                                    "XX  "
                                                    " X  "
                                                    "    "
};

size_t const tetromino_rotate_sizes[NUM_TETROMINO_TYPES]
    = {3, 3, 3, 2, 4, 3, 3};

Color const tetromino_colors[NUM_TETROMINO_TYPES] = {
    RED,
    YELLOW,
    GREEN,
    LIME,
    PURPLE,
    GOLD,
    SKYBLUE
};

// Private Functions //////////////////////////////////////////////////////////

static inline TetrominoType _random_tetromino_type(void) {
    return GetRandomValue(first_random_tetromino, last_random_tetromino);
}

inline void _init_tetromino_block_positions(Tetromino *const tetromino) {
    size_t position_index = 0;
    char const*const template = (tetromino_templates[tetromino->type]);
    for (size_t i = 0; i < BLOCKS_SIZE - 1; ++i) {
        if (template[i] == 'X') {
            ldiv_t const divmod = ldiv(i, EDGE_SIZE);

            tetromino->positions[position_index][X_AXIS]
                = divmod.rem;

            tetromino->positions[position_index][Y_AXIS]
                = divmod.quot;

            position_index++;
        }
        if (position_index >= NUM_TETROMINO_BLOCKS) break;
    }
} 

inline Tetromino _new_tetromino(TetrominoType type) { 
    long const y_offset = type == T_PIECE? -1L : 0L;
    Tetromino tetromino = {
        .x = COLS / 2 - 1,
        .y = y_offset,
        .rotation = 0,
        .type = type,
        .positions = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}
    };

    _init_tetromino_block_positions(&tetromino); 
    return tetromino;
}

// NOTE: Only use these for rotation adjustment
inline void _move_tetromino_relative_up(size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i)
        positions[i][Y_AXIS]--; 
}
inline void _move_tetromino_relative_down(size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i)
        positions[i][Y_AXIS]++;
}
inline void _move_tetromino_relative_left(size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i)
        positions[i][X_AXIS]--; 
}
inline void _move_tetromino_relative_right(size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i)
        positions[i][X_AXIS]++; 
}

inline void _handle_2_by_3_piece_reajustment(
    Tetromino *const tetromino,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    if (tetromino->rotation == 0) {
        _move_tetromino_relative_up(positions);
    }
    if (tetromino->rotation == 2) {
        _move_tetromino_relative_up(positions);
    }
}

inline void _handle_I_piece_reajustment(
    Tetromino *const tetromino,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    if (tetromino->rotation == 0) {
        _move_tetromino_relative_up(positions);
        _move_tetromino_relative_left(positions);
    }
    if (tetromino->rotation == 1) {
        _move_tetromino_relative_up(positions);
    }
    if (tetromino->rotation == 2) {
        _move_tetromino_relative_up(positions);
        _move_tetromino_relative_left(positions);
    }
    if (tetromino->rotation == 3) {
        _move_tetromino_relative_up(positions);
    } 
}

void _rotate_tetromino(
    Tetromino *const tetromino,
    TetrominoType board[ROWS][COLS]
) {

    // The O piece only has a rotate of 2 and rotating it doesn't do anything anyway so skip.
    if (tetromino->type == O_PIECE) return;

    size_t temp_positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
        = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};

    size_t const r = tetromino_rotate_sizes[tetromino->type];
    // size_t const x = tetromino->x;
    // size_t const y = tetromino->y;

    // Rotation Part
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) { 
        size_t a = tetromino->positions[i][X_AXIS];
        size_t temp_a = a;
        size_t b = tetromino->positions[i][Y_AXIS];
        a = b;
        b = (r - 1) - temp_a;
        temp_positions[i][X_AXIS] = a;
        temp_positions[i][Y_AXIS] = b;
    }

    // Some pieces need their relative poistion reajusting depending on rotation phase
    switch (tetromino->type) {
        case L_PIECE: {
            _handle_2_by_3_piece_reajustment(tetromino, temp_positions);
            break;
        }

        case J_PIECE: {
            _handle_2_by_3_piece_reajustment(tetromino, temp_positions);
            break;
        }

        // The T and O pieces rotates normally
        case T_PIECE: break; 
        case O_PIECE: break; 
        
        case I_PIECE: {
            _handle_I_piece_reajustment(tetromino, temp_positions);
            break;
        }

        case Z_PIECE: {
            _handle_2_by_3_piece_reajustment(tetromino, temp_positions);
            break;
        }
        case S_PIECE: {
            _handle_2_by_3_piece_reajustment(tetromino, temp_positions);
            break;
        }

        // Hopefully not
        case NO_TETROMINO: {
            fprintf(stderr, "Error: when calling _rotate_tetromino, found NO_TETROMINO");
            exit(1);
            break;
        }
    }    

    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        size_t const block_x = temp_positions[i][X_AXIS] + tetromino->x;
        size_t const block_y = temp_positions[i][Y_AXIS] + tetromino->y;
        if (block_x < 0 || block_x >= COLS || block_y >= ROWS) return;
        if (board[block_y][block_x] != NO_TETROMINO) return; 
    }

    
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        tetromino->positions[i][X_AXIS] = temp_positions[i][X_AXIS];
        tetromino->positions[i][Y_AXIS] = temp_positions[i][Y_AXIS];
    }

    tetromino->rotation++;
    if (tetromino->rotation >= MAX_NUM_ROTATIONS) {
        tetromino->rotation = 0;
    }
}

inline void _disp_current_tetromino(Tetromino const*const restrict tetromino) {
    Color const color = tetromino_colors[tetromino->type]; 
    size_t x_offset = tetromino->x;    
    size_t y_offset = tetromino->y;

    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        size_t x_relative = tetromino->positions[i][X_AXIS];
        size_t y_relative = tetromino->positions[i][Y_AXIS];
        DrawRectangle(
            (x_offset + x_relative) * BLOCK_SCALE + X_OFFSET,
            (y_offset + y_relative) * BLOCK_SCALE + Y_OFFSET,
            BLOCK_SCALE,
            BLOCK_SCALE,
            color
        );
    }
}

inline void _disp_boarders(void) {
    size_t const cols_mul = COLS * BLOCK_SCALE + X_OFFSET;
    size_t const rows_mul = ROWS * BLOCK_SCALE + Y_OFFSET; 
    DrawLine(X_OFFSET, Y_OFFSET, cols_mul, Y_OFFSET, BLACK);
    DrawLine(X_OFFSET, Y_OFFSET, X_OFFSET, rows_mul, BLACK);
    DrawLine(cols_mul, Y_OFFSET, cols_mul, rows_mul, BLACK);
    DrawLine(X_OFFSET, rows_mul, cols_mul, rows_mul, BLACK);  
}

// Exposed Functions //////////////////////////////////////////////////////////
GameState init_gamestate(void) {
    GameState game_state = {
        .current_tetromino = _new_tetromino(_random_tetromino_type()),
        .next_tetromino = _random_tetromino_type(),
        .level = 1,
        .score = 0 
    };

    for (size_t y = 0; y < ROWS; ++y) {
        for (size_t x = 0; x < COLS; ++x) {
            game_state.board[y][x] = NO_TETROMINO;
        }
    }

    return game_state;
}
  
void next_gamestate(GameState *const game_state) {
    char s[20];
    sprintf(s, "%ld", game_state->current_tetromino.rotation);
    DrawText(s, 20, 20, 20, BLACK);
    
    if (IsKeyPressed(KEY_W))  {
        _rotate_tetromino(
            &game_state->current_tetromino,
            game_state->board
        );
    }
}

void display_game(GameState const*const game_state) {
    BeginDrawing();

    ClearBackground(GRAY);
    _disp_current_tetromino(&game_state->current_tetromino);
    _disp_boarders();
    
    EndDrawing();
}
