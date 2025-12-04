#include "sync.h"

// simple test and set spinlock, multiprocessor safe enough for QEMU

void spinlock_init(spinlock_t *l) {
    l->locked = 0;
}

void spinlock_acquire(spinlock_t *l) {
    // __sync_lock_test_and_set returns old value,
    // we spin while it was 1
    while (__sync_lock_test_and_set(&l->locked, 1)) {
        // busy wait
    }
}

void spinlock_release(spinlock_t *l) {
    __sync_lock_release(&l->locked);
}

