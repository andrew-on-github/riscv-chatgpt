#include "uart.h"

#define UART0_BASE 0x10000000UL
#define UART_RHR   0x00  // receive holding reg
#define UART_THR   0x00  // transmit holding reg
#define UART_LSR   0x05  // line status reg
#define LSR_RX_READY 0x01
#define LSR_TX_IDLE  0x20

static inline uint8_t mmio_read8(uint64_t addr) {
    return *(volatile uint8_t *)addr;
}

static inline void mmio_write8(uint64_t addr, uint8_t val) {
    *(volatile uint8_t *)addr = val;
}

void uart_init(void) {
    // For QEMU virt default UART, we can often get away with no config:
    // it comes up at a sane baud, just use it.
}

void uart_putc(char c) {
    if (c == '\n') {
        uart_putc('\r');
    }
    // wait for TX ready
    while ((mmio_read8(UART0_BASE + UART_LSR) & LSR_TX_IDLE) == 0)
        ;
    mmio_write8(UART0_BASE + UART_THR, (uint8_t)c);
}

char uart_getc(void) {
    // busy wait for character
    while ((mmio_read8(UART0_BASE + UART_LSR) & LSR_RX_READY) == 0)
        ;
    return (char)mmio_read8(UART0_BASE + UART_RHR);
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

