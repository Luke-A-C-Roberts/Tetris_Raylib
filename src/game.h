#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

// 4x4 + 1 for string terminator \0
// These need to be macros for copy into array declarations
#define BLOCKS_SIZE            (size_t) 17
#define NUM_TETROMINO_TYPES    (size_t) 7
#define NUM_AXIS               (size_t) 2
#define NUM_TETROMINO_BLOCKS   (size_t) 4 
#define X_AXIS                 (size_t) 0
#define Y_AXIS                 (size_t) 1
#define EDGE_SIZE              (size_t) 4
#define MAX_NUM_ROTATIONS      (size_t) 4 
#define ROWS                   (size_t) 20
#define COLS                   (size_t) 10
#define X_OFFSET               (size_t) 100
#define Y_OFFSET               (size_t) 50
#define BLOCK_SCALE            (size_t) 16 
#define INFO_X_OFFSET          (size_t) 20
#define INFO_Y_OFFSET          (size_t) 20
#define INFO_FONT_SIZE         (size_t) 20
#define INFO_NEXT_ITEM_X       (size_t) 40
#define AUTOSHIFT_FRAMES_DELAY (size_t) 20
#define AUTOSHIFT_FRAMESKIP    (size_t) 3
#define NEW_BLOCK_POSITIONS  {{0, 0}, {0, 0}, {0, 0}, {0, 0}}

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
    size_t line_num;
    size_t lines;
    size_t total_lines;

    unsigned long long frame_number;
    size_t wait_time;

    bool deposite_on_next_frame;
    bool delayed_autoshift_pressed_down;
    size_t delayed_autoshift_frames;

} GameState;

typedef GameState (*init_gamestate_t)(size_t);
typedef void (*next_gamestate_t)(GameState*);
typedef void (*display_game_t)(GameState*);
  
#endif //GAME_H 
