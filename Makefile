CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -Wno-unused-but-set-variable
LIBS = -lncurses
TARGET = program
SRCS = program.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET) *.o tetris_save.dat tetris_save.txt Tetris_scores.txt

.PHONY: all clean
