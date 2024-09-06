#include "game.h"
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>

// Display Functions ////////////////////////////////////////////////////////// 
static Color const _tetromino_colors_default[NUM_TETROMINO_TYPES + 1]
    = {RED, YELLOW, GREEN, BLUE, PURPLE, GOLD, SKYBLUE}; 
  

static inline void _disp_current_tetromino_default(
    Tetromino const*const tetromino,
    size_t const border_x_offset,
    size_t const border_y_offset,
    Color const tetromino_colors[NUM_TETROMINO_TYPES + 1]
) {
    Color const color = tetromino_colors[tetromino->type]; 
    size_t const x_offset = tetromino->x;    
    size_t const y_offset = tetromino->y;

    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        size_t const x_absolute = tetromino->positions[i][X_AXIS] + x_offset;
        size_t const y_absolute = tetromino->positions[i][Y_AXIS] + y_offset;
        DrawRectangle(
            x_absolute * BLOCK_SCALE + border_x_offset,
            y_absolute * BLOCK_SCALE + border_y_offset,
            BLOCK_SCALE,
            BLOCK_SCALE,
            color
        );
    }
}

static inline void _disp_borders_default(
    size_t const border_x,
    size_t const border_y,
    size_t const border_x_offset,
    size_t const border_y_offset
) {
    DrawLine(border_x_offset, border_y_offset,
             border_x, border_y_offset, BLACK);

    DrawLine(border_x_offset, border_y_offset,
             border_x_offset, border_y, BLACK);

    DrawLine(border_x, border_y_offset,
             border_x, border_y, BLACK);

    DrawLine(border_x_offset, border_y,
             border_x, border_y, BLACK);  
}

static inline void _disp_blocks_default(
    TetrominoType const board[ROWS][COLS],
    size_t const border_x_offset,
    size_t const border_y_offset,
    Color const tetromino_colors[NUM_TETROMINO_TYPES + 1]
) {
    for (size_t y = 0; y < ROWS; ++y) {
        for (size_t x = 0; x < COLS; ++x) DrawRectangle(
            x * BLOCK_SCALE + border_x_offset,
            y * BLOCK_SCALE + border_y_offset,
            BLOCK_SCALE,
            BLOCK_SCALE,
            tetromino_colors[board[y][x]]
        );
    }
} 

static inline void _disp_info(
    GameState const*const game_state,
    Color const font_color
) {
    static char temp_str[20];
    sprintf(temp_str, "%zu", game_state->level);
    DrawText(
        temp_str,
        INFO_X_OFFSET,
        INFO_Y_OFFSET,
        INFO_FONT_SIZE,
        font_color
    );

    sprintf(temp_str, "%zu", game_state->score);
    DrawText(
        temp_str,
        INFO_X_OFFSET + INFO_NEXT_ITEM_X * 1,
        INFO_Y_OFFSET,
        INFO_FONT_SIZE,
        font_color
    ); 
}

static inline void _draw_wireframe_block(
    Color const color,
    size_t const x,
    size_t const y,
    unsigned char show_side_bitmap
) { 
    bool const show_up    = show_side_bitmap & 0x01;
    bool const show_down  = show_side_bitmap & 0x02;
    bool const show_left  = show_side_bitmap & 0x04;
    bool const show_right = show_side_bitmap & 0x08;
    
    if (show_up) DrawLineEx(
        (Vector2){
            x + LINE_THICKNESS + (show_left? 0 : -2 * LINE_THICKNESS),
            y + LINE_THICKNESS
        },
        (Vector2){
            x + BLOCK_SCALE - LINE_THICKNESS + (show_right? 0 : 2 * LINE_THICKNESS),
            y + LINE_THICKNESS
        },
        (float)LINE_THICKNESS, color
    );

    if (show_down) DrawLineEx(
        (Vector2){
            x + LINE_THICKNESS + (show_left? 0 : -2 * LINE_THICKNESS),
            y + BLOCK_SCALE - LINE_THICKNESS
        },
            
        (Vector2){
            x + BLOCK_SCALE - LINE_THICKNESS + (show_right? 0 : 2 * LINE_THICKNESS),
            y + BLOCK_SCALE - LINE_THICKNESS
        },
            
        (float)LINE_THICKNESS, color
    );

    if (show_left) DrawLineEx(
        (Vector2){
            x + LINE_THICKNESS,
            y + LINE_THICKNESS + (show_up? 0 : -2 * LINE_THICKNESS)
        },
        (Vector2){
            x + LINE_THICKNESS,
            y + BLOCK_SCALE - LINE_THICKNESS + (show_down? 0 : 2 * LINE_THICKNESS)
        },
        (float)LINE_THICKNESS, color
    );

    if (show_right) DrawLineEx(
        (Vector2){
            x + BLOCK_SCALE - LINE_THICKNESS,
            y + LINE_THICKNESS + (show_up? 0 : -2 * LINE_THICKNESS)
        },
        (Vector2){
            x + BLOCK_SCALE - LINE_THICKNESS,
            y + BLOCK_SCALE - LINE_THICKNESS + (show_down? 0 : 2 * LINE_THICKNESS)
        },
        (float)LINE_THICKNESS, color
    );
}

static inline void _disp_current_tetromino_wireframe(
    Tetromino const*const tetromino,
    size_t    const border_x_offset,
    size_t    const border_y_offset,
    Color     const tetromino_colors[NUM_TETROMINO_TYPES + 1]
) {
    Color const color = tetromino_colors[tetromino->type]; 
    size_t const x_offset = tetromino->x;    
    size_t const y_offset = tetromino->y;

    for (size_t i = 0; i < NUM_TETROMINO_BLOCKS; ++i) {
        bool show_up    = true;
        bool show_down  = true;
        bool show_left  = true;
        bool show_right = true;
        
        size_t const x = tetromino->positions[i][X_AXIS];
        size_t const y = tetromino->positions[i][Y_AXIS];
        for (size_t j = 0; j < NUM_TETROMINO_BLOCKS; ++j) {
            if (i == j) continue;
            size_t const other_x = tetromino->positions[j][X_AXIS];
            size_t const other_y = tetromino->positions[j][Y_AXIS];
            if (other_y == y - 1 && other_x == x) show_up    = false;
            if (other_y == y + 1 && other_x == x) show_down  = false;
            if (other_x == x - 1 && other_y == y) show_left  = false;
            if (other_x == x + 1 && other_y == y) show_right = false;
        }
        
        size_t const x_absolute = x + x_offset;
        size_t const y_absolute = y + y_offset;

        _draw_wireframe_block(color,
            x_absolute * BLOCK_SCALE + border_x_offset,
            y_absolute * BLOCK_SCALE + border_y_offset,
              show_up * 0x01
            + show_down * 0x02
            + show_left * 0x04
            + show_right * 0x08
        );
    } 
} 

static inline void _disp_borders_wireframe(
    size_t border_x,
    size_t border_y,
    size_t border_x_offset,
    size_t border_y_offset 
) {
    border_x += 2;
    border_y += 2;
    border_x_offset -= 2;
    border_y_offset -= 2;
    DrawLineEx((Vector2){border_x_offset, border_y_offset},
               (Vector2){border_x, border_y_offset},
               2.0, WHITE);

    DrawLineEx((Vector2){border_x_offset, border_y_offset},
               (Vector2){border_x_offset, border_y},
               2.0, WHITE);

    DrawLineEx((Vector2){border_x, border_y_offset},
               (Vector2){border_x, border_y},
               2.0, WHITE);

    DrawLineEx((Vector2){border_x_offset, border_y},
               (Vector2){border_x, border_y},
               2.0, WHITE);
}
 
static inline void _disp_blocks_wireframe(
    TetrominoType const board[ROWS][COLS],
    size_t const border_x_offset,
    size_t const border_y_offset,
    Color const tetromino_colors[NUM_TETROMINO_TYPES + 1]
) {
    for (size_t y = 0; y < ROWS; ++y) {
        for (size_t x = 0; x < COLS; ++x) {
            TetrominoType tetromino_type = board[y][x];
            if (tetromino_type == NO_TETROMINO) continue;

            Color color = tetromino_colors[tetromino_type];
            bool show_up    = true;
            bool show_down  = true;
            bool show_left  = true;
            bool show_right = true;

            if (y - 1 >= 0)   show_up    = board[y-1][x] != tetromino_type;
            if (y + 1 < ROWS) show_down  = board[y+1][x] != tetromino_type;
            if (x - 1 >= 0)   show_left  = board[y][x-1] != tetromino_type;
            if (x + 1 < COLS) show_right = board[y][x+1] != tetromino_type;
            
            _draw_wireframe_block(
                color,
                x * BLOCK_SCALE + border_x_offset, 
                y * BLOCK_SCALE + border_y_offset,
                  show_up * 0x01
                + show_down * 0x02
                + show_left * 0x04
                + show_right * 0x08 
            );
        }
    } 
}

// Display Exposed ////////////////////////////////////////////////////////////
extern DisplayConfig init_display_config(DisplayMode const display_mode) {
    DisplayConfig display_config = {
        .border_width  = BLOCK_SCALE * COLS,
        .border_height = BLOCK_SCALE * ROWS
    };

    switch (display_mode) {
        case (DEFAULT_DISPLAY_MODE): {
            display_config.font_color = BLACK;
            display_config.background_color = GRAY;

            for (size_t i = 0; i < NUM_TETROMINO_TYPES; ++i)
                display_config.tetromino_colors[i]
                    = _tetromino_colors_default[i]; 

            display_config.tetromino_colors[NUM_TETROMINO_TYPES] = GRAY;

            display_config.disp_blocks = &_disp_blocks_default;
            display_config.disp_borders = &_disp_borders_default;
            display_config.disp_current_tetromino
                = &_disp_current_tetromino_default;
            display_config.disp_info = &_disp_info; 
            break;
        }
        // TODO: different display
        case (WIREFRAME_DISPLAY_MODE): {
            display_config.font_color = WHITE;
            display_config.background_color = (Color) {26, 29, 40, 255};

            for (size_t i = 0; i < NUM_TETROMINO_TYPES; ++i)
                display_config.tetromino_colors[i]
                    = _tetromino_colors_default[i];

            display_config.tetromino_colors[NUM_TETROMINO_TYPES]
                = (Color) {26, 29, 40, 255};
  
            display_config.disp_blocks = &_disp_blocks_wireframe;
            display_config.disp_borders = &_disp_borders_wireframe;
            display_config.disp_current_tetromino
                = &_disp_current_tetromino_wireframe;
            display_config.disp_info = &_disp_info;
            break;
        }
    } 
    return display_config;
}

extern void display_game(
    GameState     const*const game_state,
    DisplayConfig const*const display_config
) {
    size_t const screen_height = GetScreenHeight();
    size_t const screen_width  = GetScreenWidth();

    size_t const border_x_offset
        = screen_width / 2 - display_config->border_width / 2;

    size_t const border_y_offset
        = screen_height / 2 - display_config->border_height / 2;

    size_t const border_x  = display_config->border_width + border_x_offset;
    size_t const boarder_y = display_config->border_height + border_y_offset;

    BeginDrawing();

    ClearBackground(display_config->background_color);

    display_config->disp_blocks(
        game_state->board,
        border_x_offset,
        border_y_offset,
        display_config->tetromino_colors
    );

    display_config->disp_borders(
        border_x,
        boarder_y,
        border_x_offset,
        border_y_offset
    );

    display_config->disp_current_tetromino(
        &game_state->current_tetromino,
        border_x_offset,
        border_y_offset,
        display_config->tetromino_colors
    );

    display_config->disp_info(
        game_state,
        display_config->font_color
    );
    
    EndDrawing();
}
