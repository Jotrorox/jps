CC = gcc

CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -O3 -g -lSDL2 -lSDL2_ttf -lm

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

TARGET = jgs

build:
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

run:
	./$(TARGET)