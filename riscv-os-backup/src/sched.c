#include "sched.h"
#include "console.h"
#include "uart.h"
#include <stddef.h>

static task_t tasks[MAX_TASKS];
static task_t *current = 0;
static int next_pid = 1;

extern void context_switch(context_t *old, context_t *new);

// simple bump allocator for stacks
extern uint8_t __bss_end;
static uint8_t *heap_ptr = &__bss_end;

static void *kalloc(size_t bytes) {
    // align
    bytes = (bytes + 15) & ~((size_t)15);
    void *p = heap_ptr;
    heap_ptr += bytes;
    return p;
}

// When a new task is first scheduled, execution starts here.
// It calls the task's entry function with its arg, then exits.
static void task_trampoline(void) {
    if (!current || !current->entry) {
        console_puts("task_trampoline: no current task or entry\n");
        while (1) { }
    }
    current->entry(current->arg);
    // If entry returns, cleanly exit the task
    task_exit();
}


void sched_init(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_UNUSED;
    }
}

int task_spawn(void (*entry)(void *), void *arg) {
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return -1;

    task_t *t = &tasks[slot];
    t->pid   = next_pid++;
    t->state = TASK_RUNNABLE;
    t->entry = entry;
    t->arg   = arg;

    // allocate 8 KB stack
    const size_t stack_size = 8 * 1024;
    t->stack_base = (uint8_t *)kalloc(stack_size);
    t->stack_top  = t->stack_base + stack_size;

    uint64_t *sp = (uint64_t *)t->stack_top;

    // initial context: when we first switch into this task,
    // it will start executing at task_trampoline() with this stack.
    t->ctx.sp = (uint64_t)sp;
    t->ctx.ra = (uint64_t)task_trampoline;

    for (int i = 0; i < 12; i++) {
        t->ctx.s[i] = 0;
    }

    return t->pid;
}


static task_t *pick_next(void) {
    if (!current) {
        for (int i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].state == TASK_RUNNABLE) return &tasks[i];
        }
        return 0;
    }
    int start = (current - tasks + 1) % MAX_TASKS;
    for (int i = 0; i < MAX_TASKS; i++) {
        int idx = (start + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_RUNNABLE) return &tasks[idx];
    }
    return 0;
}

void task_yield(void) {
    task_t *prev = current;
    task_t *next = pick_next();
    if (!next || next == prev) return;

    if (prev && prev->state == TASK_RUNNING) {
        prev->state = TASK_RUNNABLE;
    }
    next->state = TASK_RUNNING;

    task_t *old = prev;
    current = next;

    if (old) {
        context_switch(&old->ctx, &next->ctx);
    } else {
        // first time: just jump into next
        context_switch(0, &next->ctx);
    }
}

void task_exit(void) {
    if (!current) return;
    current->state = TASK_EXITED;
    // pick another runnable task
    task_t *next = pick_next();
    if (!next) {
        console_puts("No more tasks, halting.\n");
        while (1) { }
    }
    task_t *old = current;
    current = next;
    next->state = TASK_RUNNING;
    context_switch(&old->ctx, &next->ctx);
}

void sched_start(void) {
    task_t *next = pick_next();
    if (!next) {
        console_puts("sched_start: no runnable tasks\n");
        while (1) { }
    }
    current = next;
    current->state = TASK_RUNNING;

    // first switch: old context is null
    context_switch(0, &current->ctx);
}


void sched_dump(void) {
    console_puts("PID  STATE\n");
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state != TASK_UNUSED) {
            console_puts("  ");
            // very tiny decimal printing
            int pid = tasks[i].pid;
            char buf[4];
            int idx = 0;
            if (pid == 0) buf[idx++] = '0';
            while (pid > 0 && idx < 3) {
                buf[idx++] = (char)('0' + pid % 10);
                pid /= 10;
            }
            for (int k = idx - 1; k >= 0; k--) uart_putc(buf[k]);
            console_puts("    ");
            switch (tasks[i].state) {
                case TASK_RUNNABLE: console_puts("RUNNABLE"); break;
                case TASK_RUNNING:  console_puts("RUNNING"); break;
                case TASK_EXITED:   console_puts("EXITED"); break;
                default:            console_puts("???"); break;
            }
            console_puts("\n");
        }
    }
}

