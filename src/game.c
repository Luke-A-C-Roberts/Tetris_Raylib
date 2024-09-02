#include "game.h"
#include "config.h"
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Data ///////////////////////////////////////////////////////////////////////
char const tetromino_templates[NUM_TETROMINO_TYPES][BLOCKS_SIZE] = {
    "XX  "
    " X  "
    " X  "
    "    ",
    
    "XX  "
    "X   "
    "X   "
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

unsigned char const tetromino_rotate_sizes[NUM_TETROMINO_TYPES]
    = {3, 3, 3, 2, 4, 3, 3};

Color const tetromino_colors[NUM_TETROMINO_TYPES]
    = {RED, YELLOW, GREEN, LIME, PURPLE, GOLD, SKYBLUE };

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

            tetromino->positions[position_index][X_AXIS] = divmod.rem; 
            tetromino->positions[position_index][Y_AXIS] = divmod.quot;

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
        .positions = NEW_BLOCK_POSITIONS
    };

    _init_tetromino_block_positions(&tetromino); 
    return tetromino;
}

// Tetromino Movement /////////////////////////////////////////////////////////
inline void _rotate_temp_positions(
    unsigned char const rotation_size,
    size_t const tetromino_positions[NUM_TETROMINO_BLOCKS][NUM_AXIS],
    size_t temp_positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) { 
        size_t a = tetromino_positions[i][X_AXIS];
        size_t temp_a = a;
        size_t b = tetromino_positions[i][Y_AXIS];
        a = b;
        b = (rotation_size - 1) - temp_a;
        temp_positions[i][X_AXIS] = a;
        temp_positions[i][Y_AXIS] = b;
    }  
}
 
// NOTE: Only use these for rotation adjustment
inline void _move_tetromino_relative_up(
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) positions[i][Y_AXIS]--; 
}

inline void _move_tetromino_relative_left(
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) positions[i][X_AXIS]--; 
}

inline void _handle_2_by_3_piece_reajustment(
    size_t const rotation,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    // Optimisation x in [0..4] so {x % 2 == 0} = {x == 0 or x == 2}
    if (rotation % 2 == 0) _move_tetromino_relative_up(positions);
}

inline void _handle_I_piece_reajustment(
    size_t const rotation,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) { 
    // Optimisation x in [0..4] so {x % 2 == 0} = {x == 0 or x == 2}
    _move_tetromino_relative_up(positions); 
    if (rotation % 2 == 0) _move_tetromino_relative_left(positions); 
}

void _handle_rotation_adjustment(
    TetrominoType const type,
    size_t const rotation,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS] 
) {
    // Some pieces need their relative poistion reajusting depending on rotation
    // phase.
    switch (type) {
        case L_PIECE: {
            _handle_2_by_3_piece_reajustment(rotation, positions);
            break;
        }

        case J_PIECE: {
            _handle_2_by_3_piece_reajustment(rotation, positions);
            break;
        }

        // The T and O pieces rotates normally
        case T_PIECE: break; 
        case O_PIECE: break; 
        
        case I_PIECE: {
            _handle_I_piece_reajustment(rotation, positions);
            break;
        }

        case Z_PIECE: {
            _handle_2_by_3_piece_reajustment(rotation, positions);
            break;
        }
        case S_PIECE: {
            _handle_2_by_3_piece_reajustment(rotation, positions);
            break;
        }

        // Hopefully not
        case NO_TETROMINO: {
            fprintf(
                stderr,
                "Error: when calling _rotate_tetromino, found NO_TETROMINO.\n"
            );
            exit(1);
            break;
        }
    }     
}

// See definition in Events section
bool _has_tetromino_collided(
    size_t const x,
    size_t const y,
    size_t const positions[NUM_TETROMINO_BLOCKS][NUM_AXIS],
    TetrominoType const board[ROWS][COLS] 
);

void _rotate_tetromino(
    Tetromino *const tetromino,
    TetrominoType const board[ROWS][COLS]
) {

    // relative rotation simulated in temp_positions
    size_t temp_positions[NUM_TETROMINO_BLOCKS][NUM_AXIS] = NEW_BLOCK_POSITIONS;
    unsigned char const rotation_size = tetromino_rotate_sizes[tetromino->type];
    _rotate_temp_positions(
        rotation_size,
        tetromino->positions,
        temp_positions
    );

    // an adjustment needs to be made for some pieces after rotation depending,
    // on current rotation state.
    _handle_rotation_adjustment(
        tetromino->type,
        tetromino->rotation,
        temp_positions
    );

    // now we check the simulated positions to see if they have colided with a
    // block or a wall. If they have then abort rotation.
    if(_has_tetromino_collided(
        tetromino->x,
        tetromino->y,
        temp_positions,
        board
    )) return; 
 
    // Copy the simulated positions as tetromino positions
    size_t const coordinate_size = 2UL * sizeof(size_t);
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i)
        memcpy(tetromino->positions[i], temp_positions[i], coordinate_size);

    // Update rotation num and wrap to 0 if it becomes 4
    tetromino->rotation++;
    tetromino->rotation %= 4;
}

void _move_tetromino(
    MoveDirection const move_direction,
    Tetromino *const tetromino,
    TetrominoType const board[ROWS][COLS] 
) {
    size_t new_x = tetromino->x;
    size_t new_y = tetromino->y;

    switch (move_direction) {
        case MOVE_UP   : return;
        case MOVE_DOWN : new_y++; break;
        case MOVE_LEFT : new_x--; break;
        case MOVE_RIGHT: new_x++; break; 
    }

    if(_has_tetromino_collided(
        new_x,
        new_y,
        tetromino->positions,
        board
    )) return;
    
    tetromino->x = new_x;
    tetromino->y = new_y;
}

// Event Functions ////////////////////////////////////////////////////////////
inline bool _has_tetromino_collided(
    size_t const x,
    size_t const y,
    size_t const positions[NUM_TETROMINO_BLOCKS][NUM_AXIS],
    TetrominoType const board[ROWS][COLS] 
) { 
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        size_t const block_x = positions[i][X_AXIS] + x;
        size_t const block_y = positions[i][Y_AXIS] + y;
        if (block_x < 0 || block_x >= COLS || block_y >= ROWS) return true;
        if (board[block_y][block_x] != NO_TETROMINO) return true;
    }
    return false;
}

inline bool _has_tetromino_landed(
    size_t const x,
    size_t const y,
    size_t const positions[NUM_TETROMINO_BLOCKS][NUM_AXIS],
    TetrominoType const board[ROWS][COLS] 
) {
    if (y + 1 < ROWS) {
        for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
            size_t const block_x = positions[i][X_AXIS] + x;
            size_t const block_y = positions[i][Y_AXIS] + y + 1;
            if (board[block_y][block_x] != NO_TETROMINO) return true;
        }
        return false;
    }
    return true;
}

inline size_t _calc_wait(size_t const level) {
    if      (level < 15UL) return 51UL - (3UL * level);
    else if (level < 30UL) return 10UL - (level / 4UL);
    else if (level < 40UL) return 2UL;
    else                   return 1UL;
}  

void _handle_user_input_movement(GameState *const game_state) {
    if (IsKeyPressed(KEY_W)) _rotate_tetromino(
        &game_state->current_tetromino,
        game_state->board
    );

    else if (IsKeyPressed(KEY_A)) _move_tetromino( 
        MOVE_LEFT,
        &game_state->current_tetromino,
        game_state->board
    );

    else if (IsKeyPressed(KEY_D)) _move_tetromino( 
        MOVE_RIGHT,
        &game_state->current_tetromino,
        game_state->board
    );
    

    else if (IsKeyPressed(KEY_S)) _move_tetromino( 
        MOVE_DOWN,
        &game_state->current_tetromino,
        game_state->board
    );   
}

void _deposit_current_tetromino(GameState *const game_state) {
    Tetromino const*const tetromino = &game_state->current_tetromino;
    size_t const x_offset = tetromino->x;
    size_t const y_offset = tetromino->y;

    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        size_t const x_absolute = tetromino->positions[i][X_AXIS] + x_offset;
        size_t const y_absolute = tetromino->positions[i][Y_AXIS] + y_offset;
        game_state->board[y_absolute][x_absolute] = tetromino->type;
    }

    game_state->current_tetromino = _new_tetromino(game_state->next_tetromino);
    game_state->next_tetromino = _random_tetromino_type();
}

void _handle_tetromino_automatic_movement(GameState *const game_state) {
    // is this frame one where it moves down?
    if (game_state->frame_number % game_state->wait_time != 0) return; 

    _move_tetromino( 
        MOVE_DOWN,
        &game_state->current_tetromino,
        game_state->board
    );

    Tetromino const*const tetromino = &game_state->current_tetromino;
    if (_has_tetromino_landed(
        tetromino->x,
        tetromino->y,
        tetromino->positions,
        game_state->board
    )) _deposit_current_tetromino(game_state);
}   

bool _is_completed_row(TetrominoType const row[COLS]) {
    for (size_t x = 0; x < COLS; ++x) {
        if (row[x] == NO_TETROMINO) return false;
    } 
    return true;
}

unsigned char _count_complete_rows(TetrominoType const board[ROWS][COLS]) {
    unsigned char count = 0;
    for (size_t y = 0; y < ROWS; ++y) {
        if (_is_completed_row(board[y])) count++;
    } 
    return count;
}  

// NOTE: This isn't the sort of function that should be run every frame
//       so is checked with _count_complete_rows first.
void _remove_completed_rows(TetrominoType board[ROWS][COLS]) {

    // must be cast to signed long to avoid the while loop from going on forever
    // in the case of size_t, x >= 0 is always true
    signed long old_y = ROWS - 1;
    signed long new_y = ROWS - 1;
 
    while (old_y >= 0 && new_y >= 0) {
        if (_is_completed_row(board[old_y])) new_y--;
        // if (old_y == new_y) continue;

        for (size_t x = 0; x < COLS; ++x)
            board[old_y][x] = board[new_y][x];

        old_y--; new_y--;
    }
}

inline size_t _calc_score(size_t const level, size_t const row_num) {
    size_t score = 0;

    switch (row_num) {
        case 0: break;
        case 1: score = (level + 1) * 40  ; break;
        case 2: score = (level + 1) * 100 ; break;
        case 3: score = (level + 1) * 300 ; break;
        case 4: score = (level + 1) * 1200; break;
        default: break;
    }
    return score;
}

void _handle_completed_rows(GameState *const game_state) {
    size_t completed_rows = _count_complete_rows(game_state->board);
    if (completed_rows > 0) _remove_completed_rows(game_state->board);
    game_state->score += _calc_score(game_state->level + 1, completed_rows);
}
 
// Display Functions ////////////////////////////////////////////////////////// 
inline void _disp_current_tetromino(Tetromino const*const restrict tetromino) {
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

inline void _disp_boarders(void) {
    size_t const cols_mul = COLS * BLOCK_SCALE + X_OFFSET;
    size_t const rows_mul = ROWS * BLOCK_SCALE + Y_OFFSET; 
    DrawLine(X_OFFSET, Y_OFFSET, cols_mul, Y_OFFSET, BLACK);
    DrawLine(X_OFFSET, Y_OFFSET, X_OFFSET, rows_mul, BLACK);
    DrawLine(cols_mul, Y_OFFSET, cols_mul, rows_mul, BLACK);
    DrawLine(X_OFFSET, rows_mul, cols_mul, rows_mul, BLACK);  
}

inline void _disp_blocks(TetrominoType const board[ROWS][COLS]) {
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

// Exposed Functions //////////////////////////////////////////////////////////
GameState init_gamestate(size_t level) {
    GameState game_state = {
        .current_tetromino = _new_tetromino(_random_tetromino_type()),
        .next_tetromino = _random_tetromino_type(),
        .level = level,
        .score = 0UL,
        .frame_number = 1UL,
        .wait_time = _calc_wait(level)
    };

    for (size_t y = 0; y < ROWS; ++y) {
        for (size_t x = 0; x < COLS; ++x)
            game_state.board[y][x] = NO_TETROMINO;
    }
  
    return game_state;
}
  
void next_gamestate(GameState *const game_state) {
    _handle_user_input_movement(game_state);
    _handle_tetromino_automatic_movement(game_state); 
    _handle_completed_rows(game_state);
    game_state->frame_number++;
}

void display_game(GameState const*const game_state) {
    BeginDrawing();

    ClearBackground(GRAY);
    _disp_boarders();
    _disp_blocks(game_state->board);
    _disp_current_tetromino(&game_state->current_tetromino);
    
    EndDrawing();
}
