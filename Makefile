CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = $(shell sdl2-config --libs)
CPPFLAGS = $(shell sdl2-config --cflags)

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
