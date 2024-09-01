#ifndef GAME_H
#define GAME_H

#include "config.h"
#include <stddef.h>
#include <time.h>

// 4x4 + 1 for string terminator \0
// These need to be macros for copy into array declarations
#define BLOCKS_SIZE 17
#define NUM_TETROMINO_TYPES 7
#define NUM_AXIS 2UL
#define NUM_TETROMINO_BLOCKS 4UL 
#define X_AXIS 0UL
#define Y_AXIS 1UL
#define EDGE_SIZE 4UL
#define MAX_NUM_ROTATIONS 4UL
#define NEW_BLOCK_POSITIONS {{0, 0}, {0, 0}, {0, 0}, {0, 0}}

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

typedef enum {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT
} MoveDirection;

typedef struct {
    size_t x; // must be between 0-cols
    size_t y; // must be between 0-rows
    unsigned char rotation; // must always be between 0-3
    TetrominoType type; // the actual type
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS];
} Tetromino;

typedef struct {
    Tetromino current_tetromino;
    TetrominoType next_tetromino;
    TetrominoType board[ROWS][COLS]; // to be clear TetrominoType is used for
                                     // block color
    size_t level;
    size_t score;
    unsigned long long frame_number;
    size_t wait_time;
} GameState;

typedef GameState (*init_gamestate_t)(size_t);
typedef void (*next_gamestate_t)(GameState*);
typedef void (*display_game_t)(GameState*);
  
#endif //GAME_H 
