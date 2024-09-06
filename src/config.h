#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

#define INIT_WIDTH     (size_t) 1000
#define INIT_HEIGHT    (size_t) 700
#define FPS            (size_t) 60

// DEBUG: we will make this one choosable later
#define INIT_LEVEL  (size_t) 10

typedef char const*const litstr_t;
  
litstr_t libgame_path = "build/libgame.so";

#endif // CONFIG_H
