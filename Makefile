# $@ - Target name
# $^ - Target dependencies

CC = gcc
CFLAGS = -std=c99 -Wall -Wpedantic -Wextra

release: main.c
	$(CC) $^ $(CFLAGS) -I. -I/home/joe/raylib/src \
		-I/home/joe/raylib/src/external -L. -L/home/joe/raylib/src \
		-L/home/joe/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
		-o bin/fs.o
