CROSS ?= riscv64-unknown-elf
CC    := $(CROSS)-gcc
LD    := $(CROSS)-ld
AS    := $(CROSS)-gcc
OBJCOPY := $(CROSS)-objcopy

CFLAGS := -O2 -g -nostdlib -nostartfiles -ffreestanding -Wall -Wextra -march=rv64imac -mabi=lp64 -mcmodel=medany
LDFLAGS := -T linker.ld

SRC_C := \
  src/kernel.c \
  src/uart.c \
  src/console.c \
  src/sched.c \
  src/fs.c \
  src/programs.c \
  src/shell.c \
  src/sync.c

SRC_S := \
  src/boot.S \
  src/context_switch.S

OBJ := $(SRC_C:.c=.o) $(SRC_S:.S=.o)

all: kernel.elf

kernel.elf: $(OBJ) linker.ld
	$(CC) $(CFLAGS) -Wl,$(LDFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(AS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) kernel.elf

run: kernel.elf
	qemu-system-riscv64 -machine virt -m 128M -nographic -bios none -kernel kernel.elf

.PHONY: all clean run

