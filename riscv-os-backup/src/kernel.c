#include "uart.h"
#include "console.h"
#include "sched.h"

void shell_entry(void *arg);

void kmain(void) {
    uart_init();
    console_puts("\nTiny RISC-V OS starting...\n");

    sched_init();

    int pid = task_spawn(shell_entry, 0);
    if (pid < 0) {
        console_puts("Failed to spawn shell.\n");
        while (1) { }
    }

    sched_start();  // jump into shell task, then others

    console_puts("No more tasks, halting.\n");
    while (1) { }
}

