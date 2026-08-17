# ---------------------------------------------------------------------------
#  Tank (Ed Martin, 2007) -- revival build
#
#  The whole game is one translation unit: src/tankmain.cp #includes every
#  other .cp file. So there is exactly one compile command per target.
#
#  NATIVE, original fixed-function pipeline (macOS/Linux)
#    make            build build/tank
#    make run        build and run it with the right asset path
#
#  NATIVE, through the GL1 -> GLES2 shim in src/gl1/
#    make shim       build build/tank-shim
#    make run-shim   build and run it
#
#  BROWSER (WebAssembly + WebGL)
#    make bootstrap  one-time: install the Emscripten SDK into ./emsdk
#    make web        build build/web/tank.html
#    make serve      build and serve it at http://localhost:8000/tank.html
#
#    make clean      remove build/
# ---------------------------------------------------------------------------

BIN      := build/tank
MAIN     := src/tankmain.cp

# -std=c++03 is the load-bearing flag. The 2007 source relies on pre-C++11
# rules for narrowing conversions inside aggregate initialisers; ~200 of the
# GLfloat vertex tables in GLConstants.cp are hard errors under C++11 or later.
CXXFLAGS := -std=c++03 -O2 -Wall
CXXFLAGS += -Wno-deprecated-declarations    # GLUT and the GL 1.x fixed pipeline are deprecated on macOS
CXXFLAGS += -Wno-writable-strings -Wno-unused-value

UNAME := $(shell uname -s)

ifeq ($(UNAME),Darwin)
  # The macOS SDK still ships GLUT.framework and OpenGL.framework (deprecated since 10.9,
  # still present and functional in the macOS 26 SDK). Pinned explicitly because a bare
  # `xcrun --show-sdk-path` can point at a stale symlink on Command-Line-Tools-only installs.
  SDK      := $(shell xcrun --show-sdk-path 2>/dev/null)
  ifeq ($(wildcard $(SDK)/System/Library/Frameworks/GLUT.framework),)
    SDK    := $(lastword $(sort $(wildcard /Library/Developer/CommandLineTools/SDKs/MacOSX*.sdk)))
  endif
  CXXFLAGS += -isysroot $(SDK)
  LDFLAGS  := -framework GLUT -framework OpenGL
else
  # Linux / BSD: needs freeglut + libGL + libGLU
  #   Debian/Ubuntu:  sudo apt install freeglut3-dev libglu1-mesa-dev
  #   Fedora:         sudo dnf install freeglut-devel mesa-libGLU-devel
  LDFLAGS  := -lglut -lGLU -lGL -lm
endif

SRCS := $(wildcard src/*.cp) $(wildcard src/*.h) $(wildcard src/gl1/*)

.PHONY: all run shim run-shim web serve bootstrap clean

all: $(BIN)

$(BIN): $(SRCS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -x c++ $(MAIN) -o $@ $(LDFLAGS)
	@echo "built $@"

# The game fopen()s its textures by bare filename; TANK_ASSETS tells the
# patched loader in src/GLTextures.cp where to find them.
run: $(BIN)
	TANK_ASSETS=$(CURDIR)/assets ./$(BIN)

# ---------------------------------------------------------------------------
#  Native build through the GL1->GLES2 shim. Same game, same window, but every
#  fixed-function call goes through src/gl1/. This is the reference the browser
#  build is diffed against -- if it looks right here, it will look right there.
# ---------------------------------------------------------------------------
shim: build/tank-shim

build/tank-shim: $(SRCS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -DTANK_USE_GL1SHIM -Isrc \
		-x c++ $(MAIN) -x c++ src/gl1/gl1.cpp -o $@ $(LDFLAGS)
	@echo "built $@"

run-shim: build/tank-shim
	TANK_ASSETS=$(CURDIR)/assets ./build/tank-shim

# ---------------------------------------------------------------------------
#  WebAssembly. Needs the SDK from tools/setup-emsdk.sh:
#      . ./emsdk/emsdk_env.sh && make web
# ---------------------------------------------------------------------------
EMFLAGS := -std=c++03 -O2 -DTANK_USE_GL1SHIM -Isrc \
	-lglut -lGL -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
	-sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=0 \
	-sEXPORTED_RUNTIME_METHODS=ccall \
	--preload-file assets \
	-Wno-deprecated-declarations -Wno-writable-strings -Wno-unused-value

# Use em++ from PATH if the SDK is already activated, otherwise fall back to the
# copy that tools/setup-emsdk.sh drops in ./emsdk -- so a fresh clone just works
# after `make bootstrap`, with no need to source emsdk_env.sh first.
EMXX := $(shell command -v em++ 2>/dev/null || echo $(CURDIR)/emsdk/upstream/emscripten/em++)

web: build/web/tank.html

build/web/tank.html: $(SRCS) $(wildcard assets/*) web/shell.html
	@mkdir -p build/web
	@test -x "$(EMXX)" || { \
		echo "Emscripten not found. Run:  make bootstrap"; exit 1; }
	"$(EMXX)" $(EMFLAGS) --shell-file web/shell.html \
		-x c++ $(MAIN) -x c++ src/gl1/gl1.cpp -o $@
	@echo "built $@"

# One-time setup on a fresh clone: fetches the Emscripten SDK into ./emsdk
# (~1.5 GB, gitignored). Only needed for `make web`.
bootstrap:
	sh tools/setup-emsdk.sh

serve: build/web/tank.html
	@echo "http://localhost:8000/tank.html"
	@cd build/web && python3 -m http.server 8000

clean:
	rm -rf build
