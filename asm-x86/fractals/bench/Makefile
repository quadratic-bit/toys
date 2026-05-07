CC := gcc
CFLAGS := -std=c99 -O3 -Wall -Wextra -Wpedantic
LDFLAGS := -lraylib -lm

FRAMES ?= 300
PRINT ?= 1

TARGET_UNOPT := app-unopt
TARGET_OPT := app-opt

SRC_UNOPT := main-unopt.c
SRC_OPT := main-opt.c

DEFS_FRAMES := -DBENCH_FRAMES=$(FRAMES)
DEFS_ZERO := -DBENCH_ZERO_DRAW=1

ifeq ($(PRINT),1)
DEFS_PRINT := -DBENCH_PRINT=1
else
DEFS_PRINT :=
endif

all: unopt opt

unopt: $(SRC_UNOPT)
	$(CC) $(CFLAGS) $(DEFS_FRAMES) $(DEFS_PRINT) -o $(TARGET_UNOPT) $(SRC_UNOPT) $(LDFLAGS)

opt: $(SRC_OPT)
	$(CC) $(CFLAGS) -march=native $(DEFS_FRAMES) $(DEFS_PRINT) -o $(TARGET_OPT) $(SRC_OPT) $(LDFLAGS)

unopt-zero: $(SRC_UNOPT)
	$(CC) $(CFLAGS) $(DEFS_FRAMES) $(DEFS_PRINT) $(DEFS_ZERO) -o $(TARGET_UNOPT)-zero $(SRC_UNOPT) $(LDFLAGS)

opt-zero: $(SRC_OPT)
	$(CC) $(CFLAGS) -march=native $(DEFS_FRAMES) $(DEFS_PRINT) $(DEFS_ZERO) -o $(TARGET_OPT)-zero $(SRC_OPT) $(LDFLAGS)

clean:
	rm -f $(TARGET_UNOPT) $(TARGET_OPT) $(TARGET_UNOPT)-zero $(TARGET_OPT)-zero

.PHONY: all unopt opt unopt-zero opt-zero clean
