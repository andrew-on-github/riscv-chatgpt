#pragma once
#include <stdint.h>

#define MAX_TASKS 8

typedef enum {
    TASK_UNUSED,
    TASK_RUNNABLE,
    TASK_RUNNING,
    TASK_EXITED
} task_state_t;

typedef struct {
    uint64_t ra;
    uint64_t sp;
    uint64_t s[12]; // s0..s11 callee-saved registers
} context_t;

typedef struct task {
    int         pid;
    task_state_t state;
    context_t   ctx;
    uint8_t    *stack_base;
    uint8_t    *stack_top;
    void      (*entry)(void *);
    void       *arg;
} task_t;

void sched_init(void);
int  task_spawn(void (*entry)(void *), void *arg);
void task_yield(void);
void task_exit(void);
void sched_start(void);
void sched_dump(void);

