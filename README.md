# Tank (2007) — Revival

A 2007 OpenGL tank game by **Ed Martin**, written for a Core 2 Duo MacBook, recovered in 2026 and
made to build again — natively on Apple Silicon, and **in a browser via WebAssembly**.

**Status:** builds and runs on macOS/arm64, and compiles to WASM + WebGL where it is playable in
Chrome. The original fixed-function build is kept alongside as a reference.

---

## Quick start

```sh
git clone <this repo>
cd tank-2007-revival

make run              # native, original fixed-function pipeline
```

For the browser build (one-time SDK install, ~1.5 GB into `./emsdk`, gitignored):

```sh
make bootstrap        # installs the Emscripten SDK
make serve            # builds and serves http://localhost:8000/tank.html
```

`make web` picks up `em++` from `./emsdk` automatically — no need to `source emsdk_env.sh` first.

### Requirements

| Target | Needs |
|---|---|
| `make run` on macOS | Xcode Command Line Tools. `GLUT.framework` and `OpenGL.framework` are deprecated but still ship in the macOS 26 SDK. |
| `make run` on Linux | `freeglut3-dev libglu1-mesa-dev` (Debian/Ubuntu) or `freeglut-devel mesa-libGLU-devel` (Fedora). |
| `make web` | Only `make bootstrap`. Nothing else. |

### Controls

| Input | Action |
|---|---|
| `W` `A` `S` `D` | Drive the tank tracks |
| Mouse | Aim the turret |
| Space | Fire |
| Click | Pick difficulty on the start screen |
| `P` | Pause · `1` top-down view · `0` back to normal · `+` `-` mouse sensitivity |

---

## All build targets

```
make            native, original fixed-function GL      -> build/tank
make run        …and run it

make shim       native, through the GL1->GLES2 shim     -> build/tank-shim
make run-shim   …and run it

make bootstrap  one-time: install Emscripten into ./emsdk
make web        WebAssembly + WebGL                     -> build/web/tank.html
make serve      …and serve it on :8000

make clean
```

Three builds sounds like a lot; it is deliberate. `build/tank` is the reference — the 2007 rendering
path, unchanged. `build/tank-shim` runs the same game through the new shim but in a native window
where a real debugger works. `build/web` is the same shim on WebGL. Any rendering bug can be
bisected by asking which of the three it appears in.

---

## What this is

A third-person arena shooter. You drive a tank across a 66×127 tile map, trip ambush triggers that
spawn wall turrets, collect weapon and health pickups, and eventually wake a boss — a two-part
armoured train that rolls in on rails, opens its doors, and fires at you.

Roughly 5,500 lines of C++ in 13 files, plus 17 SGI-format textures. No engine, no scene graph, no
asset pipeline, no dependency beyond GLUT. Every polygon is a hand-authored vertex table, and the
level is literally ASCII art in `GLConstants.cp`.

The author's own [`original/Readme.txt`](original/Readme.txt) is an unusually good architectural
tour, and is the reason this took hours rather than days.

---

## Layout

```
tank-2007-revival/
├── Makefile             all build targets
├── original/            pristine 2007 files, byte-for-byte. Never edited.
├── src/                 working copy — the port lives here
│   ├── Platform.h       every platform #ifdef, in one place
│   └── gl1/             the OpenGL 1.x -> GLES2/WebGL shim
├── assets/              the 17 .sgi textures
├── web/shell.html       the browser page
├── tools/               emsdk installer, overview-page builder
├── docs/
│   ├── ARCHITECTURE.md  how the game works, file by file
│   ├── FINDINGS.md      bugs and hazards found while porting
│   ├── PORTING.md       the cross-platform + WASM plan
│   └── overview.html    illustrated summary (+ .standalone.html to open directly)
└── emsdk/               NOT in git — `make bootstrap` creates it
```

`original/` and `src/` started identical. Every divergence carries a `[2026 port]` comment saying
what changed and why, so `diff -r original src` is a complete, readable changelog.

---

## Getting it to compile

Far less than nineteen years of bit-rot would suggest.

**1. Compile as C++03.** The single load-bearing flag. The game builds ~200 `GLfloat` vertex-table
entries like `{ -0.6 * ENEMY_TURRET_SIZE, ... }`, which narrow `double` → `float` inside a braced
initialiser. Legal in C++03; a hard error from C++11 on.

**2. Supply the missing `sky1.sgi`.** The archive is incomplete — the loader wants 16 textures and
this one is absent. The original called `exit(1)`, so the game died before opening a window. The
loader now degrades gracefully and substitutes `sky2.sgi`, so the sky is not quite what Ed intended.
If the file ever turns up, drop it in `assets/` and it will be used.

**3. Drop `<Carbon/Carbon.h>`**, included in two files and used in neither.

**4. Fix asset paths** — the original `fopen`ed bare filenames, so it only ran from one directory.

Plus three genuine bugs: the projection aspect ratio was hard-wired to `height/height`,
`glutInitDisplayMode` was called *after* `glutCreateWindow` where it does nothing, and
`glEnable(GL_SHADE_MODEL)` is not a valid call. All catalogued in [`docs/FINDINGS.md`](docs/FINDINGS.md).

---

## Getting it into a browser

The game is pure fixed-function OpenGL 1.1 — immediate mode, a matrix stack, and texture coordinate
generation. WebGL has none of that. [`src/gl1/`](src/gl1/) is a ~900-line shim implementing exactly
the 30-odd entry points this game uses, on top of GLES2. The game source is **unchanged**: `gl1.h`
redirects `glBegin`, `glVertex3f`, `glTexGeni` and friends by macro.

What made it tractable, and the three things worth knowing:

**There is no lighting.** Not one `glLight*` or `glMaterial*` call in the whole game. Fixed-function
lighting is normally the hardest part of a GL 1.x migration and there is simply none of it here.

**Vertices are transformed to eye space on the CPU**, at `glVertex3f` time. That one decision pays
for itself three times: the only remaining per-draw uniform is the projection matrix (so everything
batches), `GL_EYE_LINEAR` and `GL_SPHERE_MAP` texgen need eye-space position and normal anyway, and
the 2007 code has no `glPushMatrix`/`glPopMatrix` at all — every draw function manually inverts its
own transform — so there is no matrix stack to honour. The game's ~50,000 `glBegin`/`glEnd` pairs
per frame collapse into about a dozen `glDrawArrays` calls.

**`glTexImage2D` is cached by pixel pointer.** The game has no texture objects and re-uploads a full
256×256 RGBA image every time it switches texture — 15 times a frame, ~3.9 MB, to cycle between 16
images that never change. Browsers will not tolerate that. Because the game always passes the same
16 pointers, caching on the pointer turns every call after the first into a `glBindTexture`, with no
game-code change at all.

Smaller pieces: `GL_QUAD_STRIP` → `GL_TRIANGLE_STRIP` is a literal enum swap (identical vertex
ordering); `GL_POLYGON` → `GL_TRIANGLE_FAN`; `gluLookAt`/`gluPerspective` and `glutSolidSphere` are
reimplemented; `GL_DECAL` is two lines of GLSL.

### Two behaviour fixes the browser forced

**Frame timing.** The game is entirely frame-locked — every duration is `seconds * FRAME_RATE`, and
the timer was `1000/80`, which integer-divides to 12 ms (83.3 fps, not the intended 80).
`requestAnimationFrame` gives you 60 Hz or 120 Hz instead, which would run the game 26% slow or 44%
fast. `WebFrame()` in `tankmain.cp` now drives it through a fixed-timestep accumulator, so every
existing frame-counted constant stays valid.

**Difficulty buttons.** They were hit-tested against hard-coded pixel coordinates assuming the
1450×820 window. A canvas is any size, so those regions are now fractions of the current window.

---

## Verifying a rendering change

`TANK_SHOT` dumps one frame as a PPM and exits, so the shim can be checked against the original
pipeline objectively rather than by eye:

```sh
export TANK_ASSETS=$PWD/assets
TANK_SHOT=/tmp/ref.ppm  TANK_SHOT_FRAME=90 ./build/tank
TANK_SHOT=/tmp/shim.ppm TANK_SHOT_FRAME=90 ./build/tank-shim
```

At frame 90 the two agree across the whole scene apart from the start-screen panel and the sky
sphere — the sky differs because `glutSolidSphere` is retessellated by the shim.

---

## Still open

`src/MainGameRoutines.cp:151` uses `<=` on both loop bounds and writes up to 264 bytes past the end
of `Map[]` every time an ambush trigger fires. It has never visibly crashed, because `Map` is a
global and whatever follows it in BSS absorbs the damage — but it is undefined behaviour. Two
characters to fix; left alone because it is a behaviour change and therefore your call. Fifteen
other findings are in [`docs/FINDINGS.md`](docs/FINDINGS.md).

---

## Credits

Original game, engine and artwork: **Ed Martin**, September–November 2007.
SGI image loading in `GLTextures.cp`: **Silicon Graphics**, via the Xcode sample projects.
