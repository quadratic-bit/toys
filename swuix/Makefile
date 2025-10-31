BUILD_SHARED ?= 0    # set to 1 to make libswuix.so

CXX ?= g++
AR ?= ar
RM ?= rm -f
MKDIR_P ?= mkdir -p

BUILD_DIR := build
INC_PUBLIC := $(abspath headers)
INC_SRC := $(abspath src/swuix)

LIB_SRC := $(shell find src/swuix -name '*.cpp')
LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_SRC))

LIB_STATIC := $(BUILD_DIR)/libswuix.a
LIB_SHARED := $(BUILD_DIR)/libswuix.so

DIST_DIR := dist/plugin

MODE ?= debug   # debug | release

CXXFLAGS_COMMON := -std=c++03 -pedantic-errors -Wall -Wextra -Werror \
				   -Wshadow -Wformat=2 \
				   -Wmissing-declarations -Wmissing-field-initializers \
				   -Wcast-align -Wwrite-strings \
				   -Wpointer-arith -Wstrict-aliasing=2 \
				   -I$(INC_PUBLIC) -I/home/ashooww/Projects/mipt-ded-zemax/include -I$(INC_SRC)

ifeq ($(SAN),1)
	CXXFLAGS_SAN := -fsanitize=address,undefined
	LDFLAGS_SAN := -fsanitize=address,undefined
endif

CXXFLAGS_debug := -O1 -g -fstack-protector-all $(CXXFLAGS_SAN)
CXXFLAGS_release := -O2 -flto -fstack-protector-strong

SDL_CFLAGS  := $(shell pkg-config --cflags sdl3)
SDL_LDLIBS  := $(shell pkg-config --libs sdl3 sdl3-gfx sdl3-ttf)
LD_LIBRARY_PATH ?= /usr/local/lib

CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_$(MODE)) $(SDL_CFLAGS)
DEPFLAGS := -MMD -MP

CXXFLAGS_CXX17 := $(filter-out -std=c++03,$(CXXFLAGS)) -std=c++17

EXAMPLE_OBJ := $(BUILD_DIR)/extern_example.o
EXAMPLE_SRC := src/extern/example.cpp
EXAMPLE_BIN := $(BUILD_DIR)/extern_example

SDL_CORE_LIBDIR := $(shell pkg-config --variable=libdir sdl3 2>/dev/null)
SDL_TTF_LIBDIR  := $(shell pkg-config --variable=libdir sdl3-ttf 2>/dev/null)
SDL_GFX_LIBDIR  := $(shell pkg-config --variable=libdir sdl3-gfx 2>/dev/null)

PLUGIN_SRC  := src/extern/plugin.cpp
PLUGIN_SO   := $(BUILD_DIR)/libswuix_sdl3.so

RPATH_ORIGIN := -Wl,-rpath,'$$ORIGIN'

.PHONY: all clean distclean example run-example plugin plugin-dist

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

example: $(EXAMPLE_BIN)

$(EXAMPLE_OBJ): $(EXAMPLE_SRC) | $(BUILD_DIR)
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS_CXX17) $(DEPFLAGS) -c $< -o $@

$(EXAMPLE_BIN): $(EXAMPLE_OBJ)
	$(CXX) -o $@ $^ -ldl

run-example: $(EXAMPLE_BIN)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) ./$(EXAMPLE_BIN)

plugin: $(PLUGIN_SO)

plugin-dist: plugin
	$(MKDIR_P) $(DIST_DIR)
	cp -a $(PLUGIN_SO) $(DIST_DIR)/
	if [ -n "$(SDL_TTF_LIBDIR)" ];  then cp -a $(SDL_TTF_LIBDIR)/libSDL3_ttf.so* $(DIST_DIR)/; fi
	if [ -n "$(SDL_GFX_LIBDIR)" ];  then cp -a $(SDL_GFX_LIBDIR)/libSDL3_gfx.so* $(DIST_DIR)/; fi
	if [ -n "$(SDL_CORE_LIBDIR)" ]; then cp -a $(SDL_CORE_LIBDIR)/libSDL3.so*    $(DIST_DIR)/; fi
	@echo "Packed to $(DIST_DIR)"

$(PLUGIN_SO): $(PLUGIN_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS_CXX17) -fPIC $(SDL_CFLAGS) -shared -o $@ $< $(SDL_LDLIBS) $(RPATH_ORIGIN) -Wl,-soname,libswuix_sdl3.so

-include $(LIB_OBJS:.o=.d)
