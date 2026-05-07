CC       ?= gcc
TARGET    = bench
HT_OBJS  ?= ht.o ht_lookup_ext.o ht_most_frequent_index_ext.o
OBJS     := bench.o $(HT_OBJS)

CPPFLAGS ?=
CFLAGS   ?= -std=c11 -O3 -DNDEBUG -g -march=native -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Wmissing-prototypes -MMD -MP
LDFLAGS  ?=
LDLIBS   ?=
VALGRIND ?= valgrind
# valgrind --tool=callgrind --instr-atstart=no --dump-instr=yes --collect-jumps=yes --cache-sim=yes --branch-sim=yes ./bench ./data/polska-sm.txt ./data/polska-md.txt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

bench.o: bench.c ht.h
ht.o: ht.c ht.h

run: $(TARGET)
	@: $${INS:?set INS=/path/to/insert.txt}
	@: $${LOOK:?set LOOK=/path/to/lookup.txt}
	./$(TARGET) "$(INS)" "$(LOOK)"

callgrind: $(TARGET)
	@: $${INS:?set INS=/path/to/insert.txt}
	@: $${LOOK:?set LOOK=/path/to/lookup.txt}
	$(VALGRIND) --tool=callgrind --instr-atstart=no ./$(TARGET) "$(INS)" "$(LOOK)"

clean:
	rm -f $(TARGET) $(OBJS) $(OBJS:.o=.d)

.PHONY: all run callgrind clean

-include $(OBJS:.o=.d)
