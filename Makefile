CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

all: simplex

simplex: main.o simplex.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

main.o: main.c simplex.h
simplex.o: simplex.c simplex.h

clean:
	$(RM) main.o simplex.o simplex.exe simplex

.PHONY: all clean
