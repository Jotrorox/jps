CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf
CPPFLAGS = $(shell sdl2-config --cflags) -I/usr/include/SDL2 -I/usr/include/SDL2_ttf

TARGET = game
SRC = main.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

release: CFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS) -lm

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean run release
