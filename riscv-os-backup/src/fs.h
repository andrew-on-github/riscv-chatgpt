#pragma once
#include <stddef.h>

typedef struct {
    const char *name;
    char       *data;
    size_t      len;
    int         used;
} file_t;

// look up file by name, or return 0
const file_t *fs_find(const char *name);

// write or create a file with given contents
// returns number of bytes written, or -1 on error
int fs_write(const char *name, const char *data, size_t len);

// list all files, calling cb for each
void fs_list(void (*cb)(const file_t *f, void *user), void *user);

