# Compiler settings
CC = gcc
CLANG = clang
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 \
         -O3 -flto -march=native -mavx2 -ffast-math \
         -fomit-frame-pointer -pipe
DEBUG_FLAGS = -g -fsanitize=address,undefined
LIBS = -lSDL2 -lSDL2_ttf -lm

# Project files
TARGET = jgs
SOURCE = main.c
OBJECT = main.o

# Number of CPU cores for parallel builds
NPROC = $(shell nproc || echo 1)

# Default target
.DEFAULT_GOAL := all

# Main targets
all: build
	./$(TARGET)

build: $(TARGET)

clang: CC=$(CLANG)
clang: clean $(TARGET)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Linking
$(TARGET): $(OBJECT)
	$(CC) $(OBJECT) -o $(TARGET) $(LIBS) $(CFLAGS)

# Compilation
$(OBJECT): $(SOURCE)
	$(CC) $(CFLAGS) -c $(SOURCE)

# Utilities
clean:
	rm -f $(OBJECT) $(TARGET)

distclean: clean
	rm -f *~ .*.swp

# Dependencies
depend:
	$(CC) -MM $(SOURCE) > .depends

-include .depends

.PHONY: all build clean distclean depend clang debug