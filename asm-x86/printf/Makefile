CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -fno-builtin-printf -fPIE
LDFLAGS := -fPIE -Wl,--wrap=printf

all: test_printf

test_printf: main.o printf.o
	$(CC) $(LDFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

printf.o: printf.s
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test_printf main.o printf.o
