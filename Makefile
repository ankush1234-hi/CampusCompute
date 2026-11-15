CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -pthread
LDLIBS = -pthread -lrt
SRC = main.c scheduler.c banker.c logger.c ipc.c worker.c limits.c
OBJ = $(SRC:.c=.o)

campuscompute: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

%.o: %.c campus.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) campuscompute

.PHONY: clean
