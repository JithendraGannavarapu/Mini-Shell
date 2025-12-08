# Makefile for Mini UNIX Shell (Lab-1)

CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra
TARGET = shell
SRC = shell.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
