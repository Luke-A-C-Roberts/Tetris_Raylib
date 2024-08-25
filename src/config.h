#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

typedef char const*const litstr_t;

size_t const init_width = 600;
size_t const init_height = 450;

size_t const fps = 60;

size_t const rows = 20;
size_t const cols = 10;

litstr_t libgame_path = "build/libgame.so";
litstr_t draw_background_sym = "draw_background";

#endif // CONFIG_H
