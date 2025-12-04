#include "console.h"
#include "uart.h"

void console_puts(const char *s) {
    uart_puts(s);
}

static const char HEX[] = "0123456789abcdef";

void console_put_hex(unsigned long x) {
    console_puts("0x");
    for (int i = (sizeof(unsigned long) * 2) - 1; i >= 0; i--) {
        unsigned int nib = (x >> (i * 4)) & 0xf;
        uart_putc(HEX[nib]);
    }
}

int console_readline(char *buf, int maxlen) {
    int len = 0;
    while (len < maxlen - 1) {
        char c = uart_getc();
        if (c == '\r' || c == '\n') {
            uart_putc('\n');
            break;
        } else if (c == '\b' || c == 127) {
            if (len > 0) {
                len--;
                uart_putc('\b');
                uart_putc(' ');
                uart_putc('\b');
            }
        } else {
            buf[len++] = c;
            uart_putc(c);
        }
    }
    buf[len] = '\0';
    return len;
}

