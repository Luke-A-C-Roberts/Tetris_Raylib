#ifndef LOAD_H
#define LOAD_H

void *dlopen_safe(char const*const file, int const mode);
void *dlsym_safe(void *const handle, char *const name);

#endif //LOAD_H
