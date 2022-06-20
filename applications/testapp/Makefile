export PATH := /usr/lunaos/compiler/bin:$(PATH)

CC := x86_64-lunaos-gcc
LD := x86_64-lunaos-ld

SRCS = $(shell find . -type f -name '*.c')
OBJS = $(patsubst %.c, %.o, $(notdir $(SRCS)))

shell.elf: $(OBJS)
	$(LD) -g $^ --static -o $@

%.o: %.c
	$(CC) -g -c --static $< -o $@

%.o: builtins/%.c
	$(CC) -g -c --static $< -o $@
