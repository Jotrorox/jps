CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -g
LIBS = -lSDL2 -lSDL2_ttf -lm

TARGET = jgs
SOURCE = main.c
OBJECT = main.o

all: build
	./$(TARGET)

build: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) $(OBJECT) -o $(TARGET) $(LIBS)

$(OBJECT): $(SOURCE)
	$(CC) $(CFLAGS) -c $(SOURCE)

clean:
	rm -f $(OBJECT) $(TARGET)

.PHONY: all build clean