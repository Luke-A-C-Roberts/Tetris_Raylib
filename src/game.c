#include "game.h"
#include "config.h"
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Misc Calculations ////////////////////////////////////////////////////////// 

// first_line_num is calculated so that after a certain number of cleard lines,
// the level increases, at the start of the game.
static inline size_t _calc_first_line_num(size_t const start_level) {
    return 10 * start_level + 10;
} 

// calculates how many frames to skip before a tetromino moves down
static inline size_t _calc_wait(size_t const level) {
    if      (level < 15) return 51 - (3 * level);
    else if (level < 30) return 10 - (level / 4);
    else if (level < 40) return 2;
    else                 return 1;
}

// calculates a score increase depending on the number of lines cleared
static inline size_t _calc_score(size_t const level, size_t const row_num) {
    static short const score_multipliers[5] = {0, 40, 100, 300, 1200};
    return (level + 1) * score_multipliers[row_num];
}

// Tetromino Initialisation ///////////////////////////////////////////////////
static char const tetromino_templates[NUM_TETROMINO_TYPES][BLOCKS_SIZE] = {
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

static unsigned char const tetromino_rotate_sizes[NUM_TETROMINO_TYPES]
    = {3, 3, 3, 2, 4, 3, 3};

static Color const tetromino_colors[NUM_TETROMINO_TYPES]
    = {RED, YELLOW, GREEN, LIME, PURPLE, GOLD, SKYBLUE };

static inline TetrominoType _random_tetromino_type(void) {
    return GetRandomValue(first_random_tetromino, last_random_tetromino);
}

// translates the template string into relative position data when creating a
// new tetromino
static inline void _init_tetromino_block_positions(Tetromino *const tetromino) {
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

static inline Tetromino _new_tetromino(TetrominoType type) { 
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

// performs a rotation for relative position data of tetrominos
static inline void _rotate_temp_positions(
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
static inline void _move_tetromino_relative_up(
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) positions[i][Y_AXIS]--; 
}

static inline void _move_tetromino_relative_left(
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) positions[i][X_AXIS]--; 
}

static inline void _handle_2_by_3_piece_reajustment(
    size_t const rotation,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) {
    // Optimisation x in [0..4] so {x % 2 == 0} = {x == 0 or x == 2}
    if (rotation % 2 == 0) _move_tetromino_relative_up(positions);
}

static inline void _handle_I_piece_reajustment(
    size_t const rotation,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS]
) { 
    // Optimisation x in [0..4] so {x % 2 == 0} = {x == 0 or x == 2}
    _move_tetromino_relative_up(positions); 
    if (rotation % 2 == 0) _move_tetromino_relative_left(positions); 
}

// Some pieces need their relative poistion reajusting after rotating.
static void _handle_rotation_adjustment(
    TetrominoType const type,
    size_t const rotation,
    size_t positions[NUM_TETROMINO_BLOCKS][NUM_AXIS] 
) {
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
static bool _has_tetromino_collided(
    size_t const x,
    size_t const y,
    size_t const positions[NUM_TETROMINO_BLOCKS][NUM_AXIS],
    TetrominoType const board[ROWS][COLS] 
);

static void _rotate_tetromino(
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

static void _move_tetromino(
    MoveDirection const move_direction,
    Tetromino *const tetromino,
    TetrominoType const board[ROWS][COLS] 
) {
    size_t new_x = tetromino->x;
    size_t new_y = tetromino->y;

    switch (move_direction) {
        case MOVE_DOWN : new_y++; break;
        case MOVE_LEFT : new_x--; break;
        case MOVE_RIGHT: new_x++; break; 
        case MOVE_UP   : return; // this isn't expected as an option
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
static inline bool _has_tetromino_collided(
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

static inline bool _has_tetromino_landed(
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
 
static inline bool _is_completed_row(TetrominoType const row[COLS]) {
    for (size_t x = 0; x < COLS; ++x) {
        if (row[x] == NO_TETROMINO) return false;
    } 
    return true;
}

// Tetromino movement /////////////////////////////////////////////////////////
static void _handle_user_input_movement(GameState *const game_state) {
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

    // Delayed autoshift or DAS
    // after an initial press, wait and then start moving repeatedly much
    // faster. This is handled by a frame counter `delayed_autoshift_frames`
    if (IsKeyDown(KEY_A)) {
        game_state->delayed_autoshift_frames++;
        game_state->delayed_autoshift_pressed_down = false;
        if (game_state->delayed_autoshift_frames > AUTOSHIFT_FRAMES_DELAY
        &&  game_state->frame_number % AUTOSHIFT_FRAMESKIP == 0
        ) {
            _move_tetromino( 
                MOVE_LEFT,
                &game_state->current_tetromino,
                game_state->board
            );
        
        }
    }
    else if (IsKeyDown(KEY_D)) {
        game_state->delayed_autoshift_frames++;
        game_state->delayed_autoshift_pressed_down = false;
        if (game_state->delayed_autoshift_frames > AUTOSHIFT_FRAMES_DELAY
        &&  game_state->frame_number % AUTOSHIFT_FRAMESKIP == 0
        ) {
            _move_tetromino( 
                MOVE_RIGHT,
                &game_state->current_tetromino,
                game_state->board
            ); 
        }
    }
    else if (IsKeyDown(KEY_S)) {
        game_state->delayed_autoshift_frames++;
        if (game_state->delayed_autoshift_frames > AUTOSHIFT_FRAMES_DELAY
        &&  game_state->frame_number % AUTOSHIFT_FRAMESKIP == 0
        ) {
            game_state->delayed_autoshift_pressed_down = true;
            _move_tetromino( 
                MOVE_DOWN,
                &game_state->current_tetromino,
                game_state->board
            ); 
        }
    } 
    else {
        game_state->delayed_autoshift_frames = 0;
        game_state->delayed_autoshift_pressed_down = false;
    } 
}

// if the `_has_tetromino_landed` event has occured, copy the tetromino pieces
// onto the board.
static void _deposit_current_tetromino(GameState *const game_state) {
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

// after a certain number of frames, the piece should automatically move down.
static void _handle_tetromino_automatic_movement(GameState *const game_state) {
 
    // is this frame one where it moves down?
    if (game_state->frame_number % game_state->wait_time != 0) return; 

    // if the player is holding down (DAS) then we dont need to automatically
    // make the piece move down.
    if (!game_state->delayed_autoshift_pressed_down) _move_tetromino( 
        MOVE_DOWN,
        &game_state->current_tetromino,
        game_state->board
    );

    // in order to give an extra frame to the player deposite_on_next_frame
    // is used.
    Tetromino const*const tetromino = &game_state->current_tetromino;
    if (_has_tetromino_landed(
        tetromino->x,
        tetromino->y,
        tetromino->positions,
        game_state->board)
    &&  !(game_state->deposite_on_next_frame)
    ) {
        game_state->deposite_on_next_frame = true;
        return;
    }
    
    if (game_state->deposite_on_next_frame) {
        game_state->deposite_on_next_frame = false;

        // Check to see if the piece is still over something.
        // If it has then deposite the piece.
        if (_has_tetromino_landed(
            tetromino->x,
            tetromino->y,
            tetromino->positions,
            game_state->board
        )) _deposit_current_tetromino(game_state);
    }
}

size_t min(size_t const a, size_t const b) {
    return a < b? a : b;
}

size_t max(size_t const a, size_t const b) {
    return a > b? a : b;
}

// NOTE: This isn't the sort of function that should be run every frame due to computational complexity, so always check if its necessary.
static inline void _remove_completed_rows(
    TetrominoType board[ROWS][COLS],
    size_t const num_completed_rows,
    size_t const completed_rows[MAX_COMPLETED_ROWS]
) {
    // TODO: Fix this algorithm!!
    for (size_t i = 0; i < num_completed_rows; ++i) {
        size_t const completed_row_y = num_completed_rows - i - 1;
        size_t const start = i;
        size_t const end = completed_row_y;

        for (ptrdiff_t y = end; y >= start; --y) {
            for (size_t x = 0; x < COLS; ++x) {
                board[y][x] = board[max(y - 1, 0)][x];
            }
        }
    }
}  

static inline void _handle_completed_rows(GameState *const game_state) { 
    size_t num_completed_rows = 0;
    size_t completed_rows[MAX_COMPLETED_ROWS] = { 0, 0, 0, 0 };
    for (size_t y = 0; y < ROWS; ++y) {
        if (_is_completed_row(game_state->board[y])) {
            completed_rows[num_completed_rows] = y;
            num_completed_rows++;
        }
    }
    
    // TODO: Fix piece removal. perhaps get completed rows by index and pass that to `_remove_completed_rows`
    if (num_completed_rows > 0) _remove_completed_rows(
        game_state->board,
        num_completed_rows,
        completed_rows
    );

    game_state->score += _calc_score(game_state->level, num_completed_rows);
    game_state->lines += num_completed_rows;
    game_state->total_lines += num_completed_rows;
}
 
// changes the level after a certain number of rows have been cleared
// this is always 10 except the first selected level (not level 1).
static inline void _handle_level(GameState *const game_state) {
    if (game_state->lines >= game_state->line_num) {
        game_state->line_num = 10;
        game_state->lines = 0;
        game_state->level++;
    }
}
 
// Display Functions ////////////////////////////////////////////////////////// 
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

// Exposed Functions //////////////////////////////////////////////////////////
extern GameState init_gamestate(size_t level) {
    // When defining structs in c all other fields are set to 0
    GameState game_state = {
        .current_tetromino = _new_tetromino(_random_tetromino_type()),
        .next_tetromino = _random_tetromino_type(),
        .level = level,
        .line_num = _calc_first_line_num(level),
        .frame_number = 1UL,
        .wait_time = _calc_wait(level),
        .deposite_on_next_frame = false,
        .delayed_autoshift_pressed_down = false
    };

    for (size_t y = 0; y < ROWS; ++y) {
        for (size_t x = 0; x < COLS; ++x)
            game_state.board[y][x] = NO_TETROMINO;
    }
  
    return game_state;
}
  
extern bool next_gamestate(GameState *const game_state) {
    _handle_user_input_movement(game_state);
    _handle_tetromino_automatic_movement(game_state); 
    _handle_completed_rows(game_state);
    _handle_level(game_state);
    game_state->frame_number++;

    return _has_tetromino_collided(
        game_state->current_tetromino.x,
        game_state->current_tetromino.y,
        game_state->current_tetromino.positions,
        game_state->board
    );
}

extern void display_game(GameState const*const game_state) {
    BeginDrawing();

    ClearBackground(GRAY);
    _disp_boarders();
    _disp_blocks(game_state->board);
    _disp_current_tetromino(&game_state->current_tetromino);
    _disp_info(game_state);
    
    EndDrawing();
}
