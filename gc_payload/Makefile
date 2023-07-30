# What toolchain prefix should we use
# Most people who want to build this will have failkit already
CROSS ?= $(DEVKITPPC)/bin/powerpc-eabi-

# Set CC, LD, OBJCOPY based on CROSS, unless they are set already
ifeq ($(origin CC), default)
	CC := $(CROSS)gcc -m32
endif
ifeq ($(origin LD), default)
	LD := $(CROSS)ld
endif
OBJCOPY ?= $(CROSS)objcopy

CFLAGS := -Wall -W -O1 -ffreestanding -mno-eabi -mno-sdata -fno-asynchronous-unwind-tables -mcpu=750
Q :=

SOURCES := memcard_loader.c
OBJS := $(SOURCES:.c=.o)

.PHONY: usa eur
all: usa eur

usa: robohaxx-USA.bin

eur: robohaxx-EUR.bin

robohaxx-USA.bin: robohaxx-USA.elf
	@echo "  OBJCOPY   $@"
	$(Q)$(OBJCOPY) -Obinary $< $@

robohaxx-USA.elf: $(OBJS)
	@echo "  LINK      $@"
	$(Q)$(LD) -T robohaxx-USA.ld $^ -o $@

robohaxx-EUR.bin: robohaxx-EUR.elf
	@echo "  OBJCOPY   $@"
	$(Q)$(OBJCOPY) -Obinary $< $@

robohaxx-EUR.elf: $(OBJS)
	@echo "  LINK      $@"
	$(Q)$(LD) -T robohaxx-EUR.ld $^ -o $@

$(OBJS): $(SOURCES)
	@echo "  COMPILE   $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "  CLEAN"
	@-rm -f $(OBJS) robohaxx-EUR.elf robohaxx-EUR.bin robohaxx-USA.elf robohaxx-USA.bin
