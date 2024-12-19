# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11
CPPFLAGS := $(shell sdl2-config --cflags) -Iinclude
LDFLAGS := $(shell sdl2-config --libs) -lSDL2_ttf -lm

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET := $(BIN_DIR)/physics_sim

# Debug build settings
DEBUG_DIR := $(BIN_DIR)/debug
DEBUG_TARGET := $(DEBUG_DIR)/physics_sim
DEBUG_OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/debug/%.o)
DEBUG_CFLAGS := -g -O0 -DDEBUG

# Release build settings
RELEASE_DIR := $(BIN_DIR)/release
RELEASE_TARGET := $(RELEASE_DIR)/physics_sim
RELEASE_OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/release/%.o)
RELEASE_CFLAGS := -O3 -DNDEBUG

.PHONY: all clean debug release run

# Default build
all: debug

# Debug rules
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS) | $(DEBUG_DIR)
	$(CC) $(DEBUG_OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/debug
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Release rules
release: CFLAGS += $(RELEASE_CFLAGS)
release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS) | $(RELEASE_DIR)
	$(CC) $(RELEASE_OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/release/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/release
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Create directories
$(DEBUG_DIR) $(RELEASE_DIR) $(OBJ_DIR)/debug $(OBJ_DIR)/release:
	mkdir -p $@

# Run the debug build
run: debug
	$(DEBUG_TARGET)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
