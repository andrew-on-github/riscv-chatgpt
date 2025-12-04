#include "console.h"
#include "sched.h"
#include "uart.h"
#include "sync.h"
#include "fs.h"
#include "programs.h"

static volatile int shared_counter = 0;
static spinlock_t counter_lock;

void prog_addmsg(void *arg) {
    (void)arg;
    char name[16];
    char msg[128];

    console_puts("[addmsg] enter file name (max 15 chars):\n");
    console_puts("[addmsg] name> ");
    int nlen = console_readline(name, sizeof(name));
    if (nlen <= 0) {
        console_puts("[addmsg] no name, exiting\n");
        task_exit();
    }

    console_puts("[addmsg] enter message line:\n");
    console_puts("[addmsg] msg> ");
    int mlen = console_readline(msg, sizeof(msg));
    if (mlen < 0) {
        mlen = 0;
    }

    int written = fs_write(name, msg, (size_t)mlen);
    if (written < 0) {
        console_puts("[addmsg] write failed (no space or error)\n");
    } else {
        console_puts("[addmsg] wrote file '");
        for (int i = 0; i < nlen; i++) {
            uart_putc(name[i]);
        }
        console_puts("'\n");
    }

    task_exit();
}

void spawn_addmsg_task(void) {
    task_spawn(prog_addmsg, 0);
}


void prog_demo(void *arg) {
    (void)arg;
    console_puts("[demo] starting demo task\n");
    for (int i = 0; i < 10; i++) {
        console_puts("[demo] iteration ");
        char c = '0' + i;
        uart_putc(c);
        console_puts("\n");
        task_yield();
    }
    console_puts("[demo] exiting demo task\n");
    task_exit();
}

static void print_int(int x) {
    char buf[12];
    int idx = 0;
    if (x == 0) {
        uart_putc('0');
        return;
    }
    if (x < 0) {
        uart_putc('-');
        x = -x;
    }
    while (x > 0 && idx < 11) {
        buf[idx++] = '0' + (x % 10);
        x /= 10;
    }
    for (int k = idx - 1; k >= 0; k--) {
        uart_putc(buf[k]);
    }
}

void prog_counter(void *arg) {
    int id = (int)(long)arg;

    console_puts("[counter] task ");
    print_int(id);
    console_puts(" starting\n");

    for (int i = 0; i < 1000; i++) {
        spinlock_acquire(&counter_lock);
        shared_counter++;
        int local = shared_counter;
        spinlock_release(&counter_lock);

        if ((i % 250) == 0) {
            console_puts("[counter] task ");
            print_int(id);
            console_puts(" tick, shared_counter=");
            print_int(local);
            console_puts("\n");
        }

        task_yield();
    }

    console_puts("[counter] task ");
    print_int(id);
    console_puts(" done\n");
    task_exit();
}


void spawn_counter_tasks(void) {
    spinlock_init(&counter_lock);
    shared_counter = 0;
    for (int i = 0; i < 3; i++) {
        task_spawn(prog_counter, (void *)(long)i);
    }
}


// Called from shell: "run demo"
void spawn_demo_task(void) {
    task_spawn(prog_demo, 0);
}

