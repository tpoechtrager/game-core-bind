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
USE_STATIC_LUA ?= 0
LTO ?= 0
LTO_FLAGS :=

ifeq ($(LTO), 1)
  LTO_FLAGS = -flto
endif

TARGET = gcb
TARGET_CONSOLE = gcb_console.exe
TARGET_WINDOW = gcb.exe

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

LUA_INCLUDE := -Ilib/include/lua

ifeq ($(USE_STATIC_LUA), 1)
  LUA_LIB := lib/$(ARCH)/liblua.a
else
  LUA_LIB := lib/$(ARCH)/lua54.dll
endif

STATIC_FLAGS := -pthread -lws2_32

ifeq ($(CXX),)
  $(error $(ARCH)-w64-mingw32-[clang++|g++] not found in PATH)
endif

ifeq ($(CC),)
  $(error $(ARCH)-w64-mingw32-[clang|gcc] not found in PATH)
endif

ifeq ($(STRIP),)
  $(error No strip tool found in PATH)
endif

LDFLAGS_CONSOLE = $(STATIC_FLAGS) -Wl,-subsystem,console
LDFLAGS_WINDOW = $(STATIC_FLAGS) -Wl,-subsystem,windows

else

ARCH ?= $(shell uname -m)
CXX := clang++
CC := clang
STRIP := strip
LUA_INCLUDE :=
STATIC_FLAGS :=

ifeq ($(ARCH), i686)
  STATIC_FLAGS += -m32
endif
ifeq ($(ARCH), x86_64)
  STATIC_FLAGS += -m64
endif

LDFLAGS := $(STATIC_FLAGS)

LUA_LIB := -llua

endif

CXXFLAGS = -Wall -Wextra -O3 -fno-exceptions $(LTO_FLAGS) $(LUA_INCLUDE)

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
       src/window.cpp \
       src/main.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean version.lua copy-dlls

all:
	rm -f $(OBJS)
	$(MAKE) version.lua
ifeq ($(PLATFORM), mingw)
	$(MAKE) $(TARGET_CONSOLE)
	$(MAKE) $(TARGET_WINDOW)
	$(MAKE) copy-dlls
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

lib/$(ARCH)/liblua.a lib/$(ARCH)/lua54.dll:
	@echo "Building Lua $(LUA_VERSION) for $(ARCH)..."
	rm -rf lib/src
	mkdir -p lib/src lib/$(ARCH) lib/include/lua

	# Download and extract Lua source
	cd lib/src && curl -LO https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	cd lib/src && tar -xzf lua-$(LUA_VERSION).tar.gz

	# Build Lua for MinGW
	cd lib/src/lua-$(LUA_VERSION) && make mingw CC="$(CC) $(LTO_FLAGS)"

	# Copy DLL and static libraries to lib/$(ARCH)
	cp lib/src/lua-$(LUA_VERSION)/src/lua54.dll lib/$(ARCH)/
	cp lib/src/lua-$(LUA_VERSION)/src/*.a lib/$(ARCH)/ 2>/dev/null || true

	# Copy headers to lib/include/lua
	cp lib/src/lua-$(LUA_VERSION)/src/*.h lib/include/lua/

	# Optionally generate .dll.a import library for MSVC compatibility
	cd lib/$(ARCH) && \
	if command -v gendef >/dev/null 2>&1 && command -v $(ARCH)-w64-mingw32-dlltool >/dev/null 2>&1; then \
		echo "Generating dll.a import lib..."; \
		gendef lua54.dll && \
		$(ARCH)-w64-mingw32-dlltool -d lua54.def -l liblua.dll.a; \
	else \
		echo "gendef or dlltool not found, skipping dll.a generation."; \
	fi

	# Clean up temporary build directory
	rm -rf lib/src


copy-dlls:
ifeq ($(PLATFORM), mingw)
	@DLL_DIR=/usr/$(ARCH)-w64-mingw32/bin; \
	GCC_DLL=libgcc_s_seh-1.dll; \
	[ "$(ARCH)" = "i686" ] && GCC_DLL=libgcc_s_dw2-1.dll; \
	for dll in libwinpthread-1.dll libstdc++-6.dll $$GCC_DLL; do \
	  [ -f $$DLL_DIR/$$dll ] && cp $$DLL_DIR/$$dll . || echo "Skipping missing: $$DLL_DIR/$$dll"; \
	done; \
	if [ -f lib/$(ARCH)/lua54.dll ]; then \
	  cp lib/$(ARCH)/lua54.dll .; \
	else \
	  echo "Skipping missing: lib/$(ARCH)/lua54.dll"; \
	fi
endif

version.lua:
	@echo "Generating version.lua..."
	@echo "gcb.version = {" > version.lua
	@echo "  Build = $$(git rev-list --count HEAD)," >> version.lua
	@echo "  GitRev = \"$(shell git rev-parse --short HEAD)\"" >> version.lua
	@echo "}" >> version.lua

clean:
	rm -f $(OBJS) version.lua
	rm -f lua54.dll
	rm -f libwinpthread-1.dll libstdc++-6.dll libgcc_s_seh-1.dll libgcc_s_dw2-1.dll
	rm -f $(TARGET_CONSOLE) $(TARGET_WINDOW) $(TARGET)
	rm -f *.pdb