UNAME_S := $(shell uname -s)

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
  PLATFORM = mingw
else
  PLATFORM = linux
endif

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

STRIP := $(shell which $(ARCH)-w64-mingw32-strip)
LUA_INCLUDE := -Ilib/$(ARCH)/lua-$(LUA_VERSION)/src
LUA_LIB := lib/$(ARCH)/lua-$(LUA_VERSION)/src/liblua.a
STATIC_FLAGS := -static-libgcc -static-libstdc++ -pthread -lws2_32

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

endif

CXXFLAGS = -Wall -Wextra -O3 -fno-exceptions $(LUA_INCLUDE)
ifeq ($(RUN_AS_ADMIN), 1)
  CXXFLAGS += -DRUN_AS_ADMIN
endif
LDFLAGS = $(STATIC_FLAGS)

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
       src/admin.cpp \
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

$(OBJS): $(LUA_LIB)

$(TARGET): $(OBJS) $(LUA_LIB)
	$(CXX) $(OBJS) $(LUA_LIB) -o $@ $(LDFLAGS)
ifeq ($(PLATFORM), mingw)
	$(STRIP) $@
endif

%.o: %.cpp cpu.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

lib/$(ARCH)/lua-$(LUA_VERSION)/src/liblua.a:
	@echo "Building Lua $(LUA_VERSION) statically for $(ARCH)..."
	mkdir -p lib/$(ARCH)
	cd lib/$(ARCH) && curl -LO https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	cd lib/$(ARCH) && tar -xzf lua-$(LUA_VERSION).tar.gz
	cd lib/$(ARCH)/lua-$(LUA_VERSION) && make mingw CC="$(CC)"

clean:
	rm -f $(OBJS) $(TARGET)
