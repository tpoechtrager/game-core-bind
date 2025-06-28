UNAME_S := $(shell uname -s)

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
  PLATFORM = mingw
else
  PLATFORM = linux
endif

LUA_VERSION = 5.4.8

ifeq ($(PLATFORM), mingw)
  CXX := $(shell which x86_64-w64-mingw32-clang++ 2>/dev/null || which x86_64-w64-mingw32-g++)
  CC := $(shell which x86_64-w64-mingw32-clang 2>/dev/null || which x86_64-w64-mingw32-gcc)
  STRIP := $(shell which x86_64-w64-mingw32-strip)
  LUA_INCLUDE := -Ilib/lua-$(LUA_VERSION)/src
  LUA_LIB := lib/lua-$(LUA_VERSION)/src/liblua.a
  STATIC_FLAGS := -static-libgcc -static-libstdc++ -pthread
  PLATFORM_LIBS := -lws2_32
else
  CXX := clang++
  CC := clang
  STRIP := strip
  LUA_INCLUDE :=
  LUA_LIB := -llua
  STATIC_FLAGS :=
  PLATFORM_LIBS :=
endif

CXXFLAGS = -Wall -Wextra -O2 $(LUA_INCLUDE)
LDFLAGS = $(STATIC_FLAGS) $(PLATFORM_LIBS)

SRCS = src/cpu.cpp \
       src/lua.cpp \
       src/lua-bindings.cpp \
       src/games.cpp \
       src/game-watcher.cpp \
       src/desktop.cpp \
       src/tools.cpp \
       src/scheduler.cpp \
       src/console.cpp \
       src/display.cpp \
       src/network.cpp \
       src/main.cpp
OBJS = $(SRCS:.cpp=.o)

EXT :=
ifeq ($(PLATFORM), mingw)
  EXT = .exe
endif

TARGET = gcb$(EXT)

.PHONY: all clean

all:
	rm -f $(OBJS)
	$(MAKE) $(TARGET)

$(TARGET): $(OBJS) $(if $(filter mingw,$(PLATFORM)),$(LUA_LIB))
	$(CXX) $(OBJS) $(LUA_LIB) -o $@ $(LDFLAGS)
ifeq ($(PLATFORM), mingw)
	$(STRIP) $@
endif

%.o: %.cpp cpu.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

lib/lua-$(LUA_VERSION)/src/liblua.a:
	@echo "Building Lua $(LUA_VERSION) statically..."
	mkdir -p lib
	cd lib && curl -LO https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	cd lib && tar -xzf lua-$(LUA_VERSION).tar.gz
	cd lib/lua-$(LUA_VERSION) && make mingw CC="$(CC)"

clean:
	rm -f $(OBJS) $(TARGET)
