#pragma once

typedef struct {
    volatile int locked;
} spinlock_t;

void spinlock_init(spinlock_t *l);
void spinlock_acquire(spinlock_t *l);
void spinlock_release(spinlock_t *l);

