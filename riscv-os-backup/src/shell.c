#include "console.h"
#include "sched.h"
#include "fs.h"
#include "uart.h"
#include "programs.h"

// forward declarations of local helpers
static void cmd_help(void);
static void show_motd(void);
static void cmd_ls(void);
static void cmd_cat(const char *name);

// our own strcmp / strncmp (no <string.h>)
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);

// main shell loop (runs inside a task)
void shell_run(void) {
    char buf[128];

    cmd_help();
    show_motd();

    while (1) {
        console_puts("> ");
        int len = console_readline(buf, sizeof(buf));
        if (len <= 0) {
            continue;
        }
        if (!buf[0]) {
            continue;
        }

        if (!strcmp(buf, "help")) {
            cmd_help();
        } else if (!strcmp(buf, "ps")) {
            sched_dump();
        } else if (!strcmp(buf, "motd")) {
            show_motd();
        } else if (!strcmp(buf, "ls")) {
            cmd_ls();
        } else if (!strncmp(buf, "cat ", 4)) {
            cmd_cat(buf + 4);
        } else if (!strcmp(buf, "run demo")) {
            spawn_demo_task();
        } else if (!strcmp(buf, "run counter")) {
            spawn_counter_tasks();
        } else if (!strcmp(buf, "run addmsg")) {
            spawn_addmsg_task();
        } else if (!strcmp(buf, "exit")) {
            console_puts("System halted. Quit QEMU with Ctrl+C in the host.\n");
            while (1) { }
        } else {
            console_puts("Unknown command.\n");
        }

        // cooperative yield so other tasks can run
        task_yield();
    }
}

// shell task entry point used by kmain
void shell_entry(void *arg) {
    (void)arg;
    shell_run();
    console_puts("Shell exited.\n");
    task_exit();
}

// ======== command implementations ========

static void cmd_help(void) {
    console_puts("Commands:\n");
    console_puts("  help            - show this help\n");
    console_puts("  ps              - show tasks\n");
    console_puts("  motd            - show message of the day\n");
    console_puts("  ls              - list files in RAM fs\n");
    console_puts("  cat <name>      - print contents of file <name>\n");
    console_puts("  run demo        - start demo task\n");
    console_puts("  run counter     - start synchronized counter tasks\n");
    console_puts("  run addmsg      - run program to add a text file\n");
    console_puts("  exit            - halt system (Ctrl+C to quit QEMU)\n");
}

static void show_motd(void) {
    const file_t *f = fs_find("motd");
    if (!f) {
        console_puts("motd not found\n");
        return;
    }
    for (size_t i = 0; i < f->len; i++) {
        uart_putc(f->data[i]);
    }
    console_puts("\n");
}

static void cmd_ls(void) {
    void printer(const file_t *f, void *user) {
        (void)user;
        console_puts(f->name);
        console_puts("\n");
    }
    fs_list(printer, 0);
}

static void cmd_cat(const char *name) {
    // skip leading spaces in case of "cat   foo"
    while (*name == ' ') name++;

    if (!*name) {
        console_puts("Usage: cat <name>\n");
        return;
    }

    const file_t *f = fs_find(name);
    if (!f) {
        console_puts("No such file.\n");
        return;
    }
    for (size_t i = 0; i < f->len; i++) {
        uart_putc(f->data[i]);
    }
    console_puts("\n");
}

// ======== tiny string helpers ========

int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb || ca == 0 || cb == 0) {
            return ca - cb;
        }
    }
    return 0;
}

