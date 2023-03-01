.POSIX:
.PHONY: all clean

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic
CC = gcc
TARGET = bfi

all: $(TARGET)

bfi: bfi.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(TARGET)
