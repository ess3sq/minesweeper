CC=gcc
SOURCES=main.c
CCFLAGS=-ggdb -Wall -Wextra -Werror -pedantic
LDFLAGS=-lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: clean

minesweeper: $(SOURCES)
	$(CC) -o minesweeper $(SOURCES) $(LDFLAGS) $(CCFLAGS)

clean:
	rm minesweeper
