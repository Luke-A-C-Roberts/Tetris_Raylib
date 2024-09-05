#ifndef LOAD_H
#define LOAD_H

void *dlopen_safe(char const*const file, int const mode);
void *dlsym_safe(void *const handle, char *const name);

#define LOAD_FUNC(handle, name)\
name##_t name = (name##_t)dlsym_safe(handle, #name)

#define RELOAD_FUNC(handle, name)\
name = (name##_t)dlsym_safe(handle, #name) 

#endif //LOAD_H
