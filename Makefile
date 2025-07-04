UNAME_S := $(shell uname -s)

ifneq (,$(findstring MINGW,$(UNAME_S)))
  PLATFORM = mingw
else ifneq (,$(findstring MSYS,$(UNAME_S)))
  PLATFORM = mingw
else
  PLATFORM = linux
endif

$(info PLATFORM = $(PLATFORM))

LUA_VERSION = 5.4.8

ifeq ($(PLATFORM), mingw)

ARCH ?= x86_64

ifeq ($(USE_GCC), 1)
  CXX := $(shell which $(ARCH)-w64-mingw32-g++)
  CC := $(shell which $(ARCH)-w64-mingw32-gcc)
else
  CXX := $(shell which $(ARCH)-w64-mingw32-clang++ 2>/dev/null || which $(ARCH)-w64-mingw32-g++)
  CC := $(shell which $(ARCH)-w64-mingw32-clang 2>/dev/null || which $(ARCH)-w64-mingw32-gcc)
endif

STRIP := $(shell which $(ARCH)-w64-mingw32-strip 2>/dev/null || which strip)

LUA_INCLUDE := -Ilib/$(ARCH)/lua-$(LUA_VERSION)/src
LUA_LIB := lib/$(ARCH)/lua-$(LUA_VERSION)/src/liblua.a
STATIC_FLAGS := -static-libgcc -static-libstdc++ -pthread -lws2_32

ifeq ($(CXX),)
  $(error $(ARCH)-w64-mingw32-[clang++|g++] not found in PATH)
endif

ifeq ($(CC),)
  $(error $(ARCH)-w64-mingw32-[clang|gcc] not found in PATH)
endif

ifeq ($(STRIP),)
  $(error No strip tool found in PATH)
endif

EXT = .exe
TARGET_CONSOLE = gcb_console$(EXT)
TARGET_WINDOW = gcb$(EXT)
LDFLAGS_CONSOLE = $(STATIC_FLAGS) -Wl,-subsystem,console
LDFLAGS_WINDOW = $(STATIC_FLAGS) -Wl,-subsystem,windows

else

ARCH ?= $(shell uname -m)
CXX := clang++
CC := clang
STRIP := strip
LUA_INCLUDE :=
LUA_LIB := -llua
STATIC_FLAGS :=

ifeq ($(ARCH), i686)
  STATIC_FLAGS += -m32
endif
ifeq ($(ARCH), x86_64)
  STATIC_FLAGS += -m64
endif

EXT =
TARGET = gcb$(EXT)
LDFLAGS := $(STATIC_FLAGS)

endif

CXXFLAGS = -Wall -Wextra -O3 -fno-exceptions $(LUA_INCLUDE)

SRCS = src/cpu.cpp \
       src/lua.cpp \
       src/lua-bindings.cpp \
       src/games.cpp \
       src/game-watcher.cpp \
       src/desktop.cpp \
       src/tools.cpp \
       src/scheduler.cpp \
       src/display.cpp \
       src/network.cpp \
       src/admin.cpp \
       src/tray.cpp \
       src/messagebox.cpp \
       src/main.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean version.lua

all:
	rm -f $(OBJS)
	$(MAKE) version.lua
ifeq ($(PLATFORM), mingw)
	$(MAKE) $(TARGET_CONSOLE)
	$(MAKE) $(TARGET_WINDOW)
else
	$(MAKE) $(TARGET)
endif

$(OBJS): $(LUA_LIB)

$(TARGET_CONSOLE): $(OBJS) $(LUA_LIB)
	$(CXX) $(OBJS) $(LUA_LIB) -o $@ $(LDFLAGS_CONSOLE)
	$(STRIP) $@

$(TARGET_WINDOW): $(OBJS) $(LUA_LIB)
	$(CXX) $(OBJS) $(LUA_LIB) -o $@ $(LDFLAGS_WINDOW)
	$(STRIP) $@

$(TARGET): $(OBJS) $(LUA_LIB)
	$(CXX) $(OBJS) $(LUA_LIB) -o $@ $(LDFLAGS)

%.o: %.cpp cpu.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

lib/$(ARCH)/lua-$(LUA_VERSION)/src/liblua.a:
	@echo "Building Lua $(LUA_VERSION) for $(ARCH)..."
	mkdir -p lib/$(ARCH)
	cd lib/$(ARCH) && curl -LO https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	cd lib/$(ARCH) && tar -xzf lua-$(LUA_VERSION).tar.gz
	cd lib/$(ARCH)/lua-$(LUA_VERSION) && make mingw CC="$(CC)"
	cd lib/$(ARCH)/lua-$(LUA_VERSION)/src && \
	if command -v gendef >/dev/null 2>&1 && command -v $(ARCH)-w64-mingw32-dlltool >/dev/null 2>&1; then \
		echo "Generating dll.a import lib..."; \
		gendef lua54.dll && \
		$(ARCH)-w64-mingw32-dlltool -d lua54.def -l liblua.dll.a && \
		cp lua54.dll ../../../..; \
	else \
		echo "gendef or dlltool not found, skipping dll.a generation."; \
	fi

version.lua:
	@echo "Generating version.lua..."
	@echo "gcb.version = {" > version.lua
	@echo "  Build = $$(git rev-list --count HEAD)," >> version.lua
	@echo "  GitRev = \"$(shell git rev-parse --short HEAD)\"" >> version.lua
	@echo "}" >> version.lua

clean:
	rm -f $(OBJS) version.lua
ifeq ($(PLATFORM), mingw)
	rm -f $(TARGET_CONSOLE) $(TARGET_WINDOW)
else
	rm -f $(TARGET)
endif
