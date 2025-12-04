#include "fs.h"

// simple in memory fs, up to 8 files, small names and data
#define MAX_FILES      8
#define MAX_NAME_LEN   16
#define MAX_DATA_LEN   256

static file_t files[MAX_FILES];
static char   name_storage[MAX_FILES][MAX_NAME_LEN];
static char   data_storage[MAX_FILES][MAX_DATA_LEN];

static int initialized = 0;

static int names_equal(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

static void str_copy(char *dst, const char *src, int max) {
    int i = 0;
    while (i < max - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static void fs_init_once(void) {
    if (initialized) return;
    initialized = 1;

    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
        files[i].name = 0;
        files[i].data = data_storage[i];
        files[i].len  = 0;
    }

    // create motd file
    const char *motd =
    "Welcome to tiny-riscv-os.\n"
    "Try: help, ps, ls, motd, run demo, run counter, run addmsg, cat motd.\n";


    str_copy(name_storage[0], "motd", MAX_NAME_LEN);
    files[0].name = name_storage[0];
    files[0].used = 1;

    int i = 0;
    while (motd[i] && i < MAX_DATA_LEN) {
        data_storage[0][i] = motd[i];
        i++;
    }
    files[0].len = (size_t)i;
}

const file_t *fs_find(const char *name) {
    fs_init_once();
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used || !files[i].name) continue;
        if (names_equal(files[i].name, name)) {
            return &files[i];
        }
    }
    return 0;
}

int fs_write(const char *name, const char *data, size_t len) {
    fs_init_once();

    // clamp name and data lengths
    char name_buf[MAX_NAME_LEN];
    str_copy(name_buf, name, MAX_NAME_LEN);

    if (len > MAX_DATA_LEN) {
        len = MAX_DATA_LEN;
    }

    // find existing file or first free slot
    int idx = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && files[i].name &&
            names_equal(files[i].name, name_buf)) {
            idx = i;
            break;
        }
        if (!files[i].used && idx == -1) {
            idx = i;
        }
    }

    if (idx < 0) {
        // no space
        return -1;
    }

    // set name if new
    if (!files[idx].used) {
        files[idx].used = 1;
        str_copy(name_storage[idx], name_buf, MAX_NAME_LEN);
        files[idx].name = name_storage[idx];
    }

    // copy data
    for (size_t i = 0; i < len; i++) {
        data_storage[idx][i] = data[i];
    }
    files[idx].len = len;

    return (int)len;
}

void fs_list(void (*cb)(const file_t *f, void *user), void *user) {
    fs_init_once();
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && files[i].name) {
            cb(&files[i], user);
        }
    }
}

