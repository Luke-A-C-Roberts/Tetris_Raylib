#ifndef GAME_H
#define GAME_H

#include "config.h"
#include <stddef.h>

// 4x4 + 1 for string terminator \0
#define BLOCKS_SIZE 17
#define NUM_TETROMINO_TYPES 7

typedef enum {
    L_PIECE,
    J_PIECE,
    T_PIECE,
    O_PIECE,
    I_PIECE,
    Z_PIECE,
    S_PIECE,
    NO_TETROMINO,
} TetrominoType;

static TetrominoType const first_random_tetromino = L_PIECE;
static TetrominoType const last_random_tetromino  = S_PIECE;

typedef struct {
    size_t x; // must be between 0-cols
    size_t y; // must be between 0-rows
    size_t rotation; // must always be between 0-3
    TetrominoType type; // the actual type
    char blocks[BLOCKS_SIZE]; // stored transformation of the piece
} Tetromino;

typedef struct {
    Tetromino current_piece;
    TetrominoType next_piece;
    TetrominoType board[rows][cols]; // to be clear TetrominoType is used for block color
    size_t level;
    size_t score;
} GameState;

typedef GameState (*init_gamestate_t)(void);
typedef void (*next_gamestate_t)(GameState*);
typedef void (*display_game_t)(GameState*);
  
#endif //GAME_H

