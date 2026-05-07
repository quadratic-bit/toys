CC := cc
CFLAGS := -std=c99 -O3 -Wall -Wextra -Wpedantic
LDFLAGS := -lraylib -lm

TARGET := app
SRC := main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
