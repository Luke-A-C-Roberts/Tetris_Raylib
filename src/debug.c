#include "debug.h"
#include "game.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define TO_BOOL_STR(b) (b)? "true" : false

static inline void _generate_tetromino_type_str(char *const tetromino_type_str, TetrominoType const tetromino_type) {
    switch (tetromino_type) {
        case L_PIECE: strcpy(tetromino_type_str, "L_PIECE"); break;
        case J_PIECE: strcpy(tetromino_type_str, "J_PIECE"); break;
        case T_PIECE: strcpy(tetromino_type_str, "T_PIECE"); break;
        case O_PIECE: strcpy(tetromino_type_str, "O_PIECE"); break;
        case I_PIECE: strcpy(tetromino_type_str, "I_PIECE"); break;
        case Z_PIECE: strcpy(tetromino_type_str, "Z_PIECE"); break;
        case S_PIECE: strcpy(tetromino_type_str, "S_PIECE"); break;
        case NO_TETROMINO: strcpy(tetromino_type_str, "NO_TETROMINO"); break;
    } 
}

static inline char _generate_tetromino_type_char(TetrominoType const tetromino_type) {
    switch (tetromino_type) {
        case L_PIECE: return 'L';
        case J_PIECE: return 'J';
        case T_PIECE: return 'T';
        case O_PIECE: return 'O';
        case I_PIECE: return 'I';
        case Z_PIECE: return 'Z';
        case S_PIECE: return 'S';
        case NO_TETROMINO: return '-';
        default: return '?';
    } 
} 

extern void print_game_state(GameState const*const game_state) {
    fprintf(stderr, "level = %zu\n", game_state->level);
    fprintf(stderr, "score = %zu\n", game_state->score);
    fprintf(stderr, "line_num = %zu\n", game_state->line_num);
    fprintf(stderr, "lines = %zu\n", game_state->lines);
    fprintf(stderr, "total_lines = %zu\n", game_state->total_lines);  
    fprintf(stderr, "wait_time = %zu\n", game_state->wait_time);
    fprintf(stderr, "frame_number = %llu\n", game_state->frame_number);
    fprintf(stderr, "delayed_autoshift_frames = %zu\n", game_state->delayed_autoshift_frames); 
    fprintf(stderr, "deposite_on_next_frame = %s\n", TO_BOOL_STR(game_state->deposite_on_next_frame));
    fprintf(stderr, "delayed_autoshift_pressed_down = %s\n", TO_BOOL_STR(game_state->delayed_autoshift_pressed_down));

    fprintf(stderr, "\ncurrent_tetromino:\n"); 
    fprintf(stderr, "\tx = %zu\n", game_state->current_tetromino.x);
    fprintf(stderr, "\ty = %zu\n", game_state->current_tetromino.y);
    fprintf(stderr, "\trotation = %hhu\n", game_state->current_tetromino.rotation);

    char tetromino_type_str[13];
    _generate_tetromino_type_str(tetromino_type_str, game_state->current_tetromino.type);
    fprintf(stderr, "\ttype = %s\n", tetromino_type_str);
    fprintf(stderr, "\tpositions = {");
    for (size_t y = 0; y < NUM_TETROMINO_BLOCKS; ++y) fprintf(
        stderr,
        "{%zu, %zu}%s",
        game_state->current_tetromino.positions[y][X_AXIS],
        game_state->current_tetromino.positions[y][Y_AXIS],
        y < NUM_TETROMINO_BLOCKS - 1? ", " : "}\n"
    );

    _generate_tetromino_type_str(tetromino_type_str, game_state->next_tetromino);
    fprintf(stderr, "\nnext_tetromino = %s\n", tetromino_type_str);

    // TetrominoType board[ROWS][COLS];
    fprintf(stderr, "\nboard:\n");

    for (size_t y = 0; y < ROWS; ++y) {
        fprintf(stderr, "\t");
        for (size_t x = 0; x < COLS; ++x) fprintf(
            stderr,
            "%c ",
            _generate_tetromino_type_char(game_state->board[y][x])
        ); 
        fprintf(stderr, "\n");
    }  
}
