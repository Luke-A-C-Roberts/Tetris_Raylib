#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

#define ROWS        (size_t) 20
#define COLS        (size_t) 10
#define INIT_WIDTH  (size_t) 600
#define INIT_HEIGHT (size_t) 450
#define FPS         (size_t) 60
#define X_OFFSET    (size_t) 100
#define Y_OFFSET    (size_t) 50
#define BLOCK_SCALE (size_t) 16

// DEBUG: we will make this one choosable later
#define INIT_LEVEL  (size_t) 10

typedef char const*const litstr_t;
  
litstr_t libgame_path = "build/libgame.so";

#endif // CONFIG_H
