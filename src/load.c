#include "load.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>

void *dlopen_safe(char const*const file, int const mode) {
    void *lib = NULL;
    lib = dlopen(file, mode);
    assert (lib != NULL); 
    return lib;
}


void *dlsym_safe(void *restrict const handle, char *restrict const name) {
    void *ptr = NULL;
    ptr = dlsym(handle, name);
    assert (ptr != NULL);
    return ptr;
}


#define LOAD_FUNC(name)\
name##_t name = (name##_t)dlsym_safe(libgame, "##name##")

