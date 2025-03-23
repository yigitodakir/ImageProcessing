CFLAGS = -O3 -Wall -Wextra -Wpedantic

.PHONY: clean all

all: main

main: main.c window.c window_v1.c zoom.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f main


