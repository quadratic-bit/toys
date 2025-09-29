SDL ?= 1             # set to 0 to build without SDL cflags
BUILD_SHARED ?= 0    # set to 1 to make libswuix.so

CXX ?= g++
AR ?= ar
RM ?= rm -f
MKDIR_P ?= mkdir -p

BUILD_DIR := build
INC_PUBLIC := headers
INC_SRC := src/swuix

LIB_SRC := $(shell find src/swuix -name '*.cpp')
LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_SRC))

LIB_STATIC := $(BUILD_DIR)/libswuix.a
LIB_SHARED := $(BUILD_DIR)/libswuix.so

MODE ?= debug   # debug | release

CXXFLAGS_COMMON := -std=c++03 -pedantic-errors -Wall -Wextra -Werror \
				   -Wshadow -Wformat=2 \
				   -Wmissing-declarations -Wmissing-field-initializers \
				   -Wcast-align -Wwrite-strings \
				   -Wpointer-arith -Wstrict-aliasing=2 \
				   -I$(INC_PUBLIC) -I$(INC_SRC)

ifeq ($(SAN),1)
	CXXFLAGS_SAN := -fsanitize=address,undefined
	LDFLAGS_SAN := -fsanitize=address,undefined
endif

CXXFLAGS_debug := -O1 -g -fstack-protector-all $(CXXFLAGS_SAN)
CXXFLAGS_release := -O2 -flto -fstack-protector-strong

ifeq ($(SDL),1)
	SDL_CFLAGS  := $(shell pkg-config --cflags sdl3)
	SDL_LDLIBS  := $(shell pkg-config --libs sdl3 sdl3-gfx sdl3-ttf)
else
	SDL_CFLAGS  :=
	SDL_LDLIBS  :=
endif

CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_$(MODE)) $(SDL_CFLAGS)
DEPFLAGS := -MMD -MP

.PHONY: all clean distclean

all: $(LIB_STATIC)

$(LIB_STATIC): $(LIB_OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $(LIB_OBJS)

ifeq ($(BUILD_SHARED),1)

$(LIB_SHARED): CXXFLAGS += -fPIC
$(BUILD_DIR)/%.o: CXXFLAGS += -fPIC
all: $(LIB_SHARED)

$(LIB_SHARED): $(LIB_OBJS) | $(BUILD_DIR)
	$(CXX) -shared -o $@ $(LIB_OBJS) $(SDL_LDLIBS) -Wl,-soname,libswuix.so

endif

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	$(MKDIR_P) $@

clean:
	$(RM) -r $(BUILD_DIR)

distclean: clean

-include $(LIB_OBJS:.o=.d)
