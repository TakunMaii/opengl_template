.PHONY: all build run

HEADER=
ROOT_SOURCE=src/main.c src/glad.c
SOURCE=

CC=gcc
CFLAGS=-Wall -Wextra -I external/include -L external/lib -lglfw3dll

TARGET=build/main


all: run

build: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(HEADER) $(ROOT_SOURCE) $(SOURCE)
	$(CC) $(ROOT_SOURCE) $(SOURCE) -o $(TARGET) $(CFLAGS)
