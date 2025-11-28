CXX ?= g++
AR ?= ar
RM ?= rm -f
MKDIR_P ?= mkdir -p

BIN_DIR := bin
BUILD_DIR := build
INC_PUBLIC := headers
INC_SRC := src/optick

# --- swuix ---
SWUIX_DIR := ../swuix
SWUIX_INC := $(SWUIX_DIR)/headers
SWUIX_LIB := $(SWUIX_DIR)/build/libswuix.a

LIB_SRC := $(shell find src/optick -name '*.cpp')
MAIN_SRC := src/main.cpp

LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_SRC))
MAIN_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(MAIN_SRC))

LIB_STATIC := $(BUILD_DIR)/liboptick.a
DEPFILES := $(LIB_OBJS:.o=.d) $(MAIN_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

MODE ?= debug   # debug | release

CXXFLAGS_COMMON := -std=c++17 -pedantic-errors -Wall -Wextra -Werror \
				   -Wshadow -Wformat=2 \
				   -Wmissing-declarations -Wmissing-field-initializers \
				   -Wcast-align -Wwrite-strings \
				   -Wpointer-arith -Wstrict-aliasing=2 \
				   -I$(INC_PUBLIC) -I/home/ashooww/Projects/mipt-ded-zemax/include -I$(INC_SRC) -I$(SWUIX_INC)

ifeq ($(SAN),1)
	CXXFLAGS_SAN := -fsanitize=address,undefined
	LDFLAGS_SAN := -fsanitize=address,undefined
endif

CXXFLAGS_debug := -O1 -g -fstack-protector-all $(CXXFLAGS_SAN)
LDFLAGS_debug := $(LDFLAGS_SAN)

CXXFLAGS_release := -O3 -mavx2 -flto -fstack-protector-strong
LDFLAGS_release := -flto

CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_$(MODE))
LDFLAGS := $(LDFLAGS_$(MODE)) -Wl,-rpath,/usr/local/lib
#LDLIBS := -lz
LDLIBS := -L/usr/local/lib -lSDL3 -lSDL3_gfx -lSDL3_ttf

DEPFLAGS := -MMD -MP

.PHONY: all clean distclean run swuix

# build swuix first
all: $(SWUIX_LIB) $(BIN_DIR)/example

$(SWUIX_LIB):
	$(MAKE) -C $(SWUIX_DIR)

$(BIN_DIR)/example: $(LIB_STATIC) $(SWUIX_LIB) $(MAIN_OBJS) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $(MAIN_OBJS) $(LIB_STATIC) $(SWUIX_LIB) $(LDLIBS)

$(LIB_STATIC): $(LIB_OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $(LIB_OBJS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

run: $(BIN_DIR)/example
	./$(BIN_DIR)/example ../swuix/dist/plugin/libswuix_sdl3.so ../swuix/dist/plugin/libpptool_line.so

$(BIN_DIR) $(BUILD_DIR):
	$(MKDIR_P) $@

clean:
	$(RM) -r $(BUILD_DIR) $(BIN_DIR)/example

distclean: clean

-include $(DEPFILES)
