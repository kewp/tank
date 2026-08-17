# Porting plan — cross-platform and WebAssembly

Three levels of ambition. Each is independently useful; each builds on the last.

> **Where things stand:** Level 0 is done — the game builds and runs natively on macOS/arm64.
> Levels 1–3 are planned, not built. Emscripten is **not installed on this machine**
> (`emcc` not found), so nothing in the WebAssembly section has been compiled and tested.
> `tools/setup-emsdk.sh` will install it when you want to start.

---

## Level 0 — macOS native ✅ done

See the [README](../README.md). Two real changes (`-std=c++03`, missing texture fallback) plus
removing Carbon and fixing three bugs.

**This works because `GLUT.framework` and `OpenGL.framework` are still in the macOS 26 SDK.** Both
have been formally deprecated since macOS 10.9 (2013). Apple has left them in place for over a
decade, but they are frozen at OpenGL 4.1 on Intel and are emulated on Apple Silicon. Treat the
current build as a working baseline, not a destination.

---

## Level 1 — Correct and portable natively

**Effort: 1–2 days. Gets you Linux and Windows.**

### 1.1 Fix the memory bug first

`MainGameRoutines.cp:151`, two `<=` → `<`. See [FINDINGS #8](FINDINGS.md#8-out-of-bounds-write-to-map--fix-this-first).
Do this before anything else — Emscripten's heap is bounds-checked in debug builds and will trap on
it, and you do not want to be debugging that while also debugging a graphics port.

### 1.2 Add texture objects

Currently zero `glGenTextures`/`glBindTexture` calls; ~3.9 MB of texture re-uploaded per frame.
See [FINDINGS #11](FINDINGS.md#11-no-texture-objects--39-mb-re-uploaded-per-frame--performance).

```c
// once, in create_textures()
GLuint texIds[16];
glGenTextures(16, texIds);
glBindTexture(GL_TEXTURE_2D, texIds[i]);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

// at each of the 15 former glTexImage2D sites
glBindTexture(GL_TEXTURE_2D, texIds[SAND1]);
```

~30 lines. **Optional natively, mandatory for WebGL.** Do it here so it is tested on a platform where
you can still use a real debugger.

### 1.3 Replace GLUT

GLUT is the main portability wall: a macOS framework on Apple, freeglut on Linux, a DIY problem on
Windows. Replace it with **GLFW** (small, MIT, no legacy baggage) or **SDL2** (bigger, but gives you
audio and gamepads for free if the project ever grows).

The surface is tiny — 13 GLUT entry points:

| GLUT | GLFW equivalent |
|---|---|
| `glutInit` / `glutCreateWindow` / `glutInitDisplayMode` | `glfwInit`, `glfwWindowHint`, `glfwCreateWindow` |
| `glutKeyboardFunc` / `glutKeyboardUpFunc` | `glfwSetKeyCallback` (one callback, both edges) |
| `glutPassiveMotionFunc` | `glfwSetCursorPosCallback` |
| `glutMouseFunc` | `glfwSetMouseButtonCallback` |
| `glutReshapeFunc` | `glfwSetFramebufferSizeCallback` |
| `glutDisplayFunc` | *(unused in practice — see below)* |
| `glutSwapBuffers` | `glfwSwapBuffers` |
| `glutMainLoop` + `glutTimerFunc` | a `while (!glfwWindowShouldClose(w))` loop |
| `glutSolidSphere` × 7 | you write it — ~30 lines of lat/long tessellation |

Note the game already calls `display()` directly from `MainGameLoop` rather than using
`glutPostRedisplay`, so the display-callback registration is nearly vestigial. That makes the loop
conversion straightforward.

**While you are here, fix pointer capture.** `glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED)`
gives relative mouse mode and fixes the turret-stops-at-screen-edge problem the author documented
([FINDINGS #16](FINDINGS.md#16-mouse-aiming-has-no-pointer-capture-)). You need this for the browser
anyway.

### 1.4 Remaining portability items

Mostly handled already by `src/Platform.h`:

- **`M_PI`** — not standard C++; absent on MSVC. Handled.
- **`random()`** — POSIX, absent on MSVC. Mapped to `rand()` so the `random()/RAND_MAX` ratio stays
  correct (naively using the real `random()` on Windows would give ratios up to 65536 because
  `RAND_MAX` is 32767 there).
- **`snprintf`** — fine on MSVC 2015+.
- **Endianness** — the SGI loader already does runtime detection. Fine.
- **Unity build** — works everywhere, but the include order is load-bearing
  ([FINDINGS #18](FINDINGS.md#18-include-order-dependency--fragility)). Leave it alone or split it
  properly; do not half-split it.

---

## Level 2 — WebAssembly / browser

**Effort: 1–2 weeks. This is genuinely feasible.**

Fixed-function OpenGL in a browser sounds worse than it is, because of one fact that dominates
everything: **this game has no lighting.** Not one `glLight*`, `glMaterial*`, or `glNormal3f` call
outside `glutSolidSphere`. Fixed-function lighting is normally the hardest part of a GL 1.x
migration and there is none of it here. What remains is immediate-mode geometry, a matrix stack, and
texture coordinate generation — all mechanical.

### 2.1 What WebGL does not have

Full inventory of the gaps, from the API survey in [ARCHITECTURE #8](ARCHITECTURE.md#8-rendering):

| Feature | Uses | Difficulty | Approach |
|---|---|---|---|
| `GL_QUAD_STRIP` | 69 | **trivial** | Quad strips and triangle strips have **identical vertex ordering**. Literal enum swap. |
| `GL_POLYGON` | 4 | **trivial** | → `GL_TRIANGLE_FAN`. Valid for convex polygons; check all 4 sites are convex (they are simple shapes). |
| `glBegin`/`glEnd` + `glVertex3f`/`glColor3f` | 73 blocks | easy | ~150-line batching shim: accumulate into a `std::vector`, flush on `glEnd`. |
| Matrix stack: `glRotated`, `glTranslated`, `glLoadIdentity`, `glMatrixMode` | 112 | easy | ~200 lines: two 4×4 stacks and standard rotate/translate builders. |
| `gluLookAt`, `gluPerspective` | 6 | easy | Textbook, ~40 lines. GLU does not exist in Emscripten. |
| `glTexEnvi(GL_DECAL)` | 1 | easy | Two lines of GLSL — but see the note below, it matters more than it looks. |
| `glutSolidSphere` | 7 | easy | ~30 lines, lat/long tessellation. Must emit normals — sphere-mapped texgen depends on them. |
| **`glTexGeni`** | **19** | **the real work** | See below. |

### 2.2 `glTexGeni` — the actual problem, and why it is tractable

The game emits **zero** `glTexCoord2f` calls. Every texture coordinate is generated by the fixed
pipeline, in three modes:

- `GL_OBJECT_LINEAR` × 11 — ground, walls, towers
- `GL_EYE_LINEAR` × 2 — turrets
- `GL_SPHERE_MAP` × 6 — the Tube boss's chrome look

None of this exists in WebGL or GLES2.

**The key insight: the game never sets custom texgen planes.** It only ever calls
`glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, ...)`, never `glTexGenfv` to specify a plane. So all planes
are the OpenGL defaults, and the three modes collapse to:

```
GL_OBJECT_LINEAR :  s = x_object   t = y_object
GL_EYE_LINEAR    :  s = x_eye      t = y_eye
GL_SPHERE_MAP    :  u = normalize(eye-space position)
                    n = eye-space normal
                    r = u - 2n(n·u)
                    m = 2·sqrt(rx² + ry² + (rz+1)²)
                    s = rx/m + ½    t = ry/m + ½
```

All three depend **only on vertex position and normal** — both of which your immediate-mode shim
already has in hand at `glVertex3f` time. So you can compute texture coordinates **on the CPU inside
the shim** and hand WebGL a plain interleaved vertex buffer. No shader gymnastics, no uniform
plumbing, no per-mode shader variants.

Two details that will bite if missed:

- **`GL_SPHERE_MAP` needs normals**, and the game's own geometry never sets any — so the current
  normal is the default `(0,0,1)` throughout. That is not a bug to fix; it is the behaviour you must
  *reproduce*, or the boss looks different. With a constant normal the reflection still varies with
  eye position, which is why it looks like a moving sheen. `glutSolidSphere` is the one thing that
  does emit real normals, so your shim's version must too.
- **`GL_DECAL` mode means `glColor3f` is mostly ignored where textures are on.** For RGBA textures,
  DECAL is `C = Cf·(1−As) + Cs·As`. Nine of the shipped textures have a real alpha channel; the
  other eight are 3-channel sources expanded with `alpha = 255`, which makes DECAL a straight
  replace. Get this wrong and half the scene comes out tinted.

### 2.3 Two routes

#### Route A — `-sLEGACY_GL_EMULATION=1`

Emscripten ships an emulation layer for immediate mode and the matrix stack. Faster to a first
triangle.

**Risk: texgen support is the specific thing this layer is weakest at**, and it is exactly what this
game leans on for 100% of its texture coordinates. The layer is also lightly maintained and its
documented limitations are not exhaustive.

**Do not plan around this without testing it first.** Spend half a day on a spike: a single textured
quad with `GL_OBJECT_LINEAR` texgen. If it renders correctly, Route A saves you a week. If it does
not, you have lost half a day and you know to take Route B.

#### Route B — hand-written GL1 → GLES2 shim ✅ recommended

Write a ~600–900 line compatibility layer implementing exactly the 30-odd GL entry points this game
uses, on top of GLES2/WebGL. Compile the game against it **unchanged**.

```
src/gl1shim/
├── gl1_matrix.{h,cpp}     matrix stacks, gluLookAt, gluPerspective       ~200 lines
├── gl1_immediate.{h,cpp}  glBegin/glVertex3f/glColor3f → vertex buffer   ~150 lines
├── gl1_texgen.{h,cpp}     the three texgen modes, CPU-side               ~120 lines
├── gl1_texture.{h,cpp}    texture objects + GL_DECAL emulation           ~100 lines
├── gl1_shader.{h,cpp}     one vertex + one fragment shader               ~80 lines
└── gl1_glut.{h,cpp}       GLUT shim over GLFW / Emscripten               ~150 lines
```

Why this is the better investment despite being more work:

- **One rendering path everywhere.** The same shim gives you WebGL in the browser *and* modern
  OpenGL natively on macOS, Linux and Windows — which also gets you off deprecated
  `GLUT.framework`/`OpenGL.framework`, solving Level 1 at the same time.
- **You control the behaviour.** When the boss's chrome looks wrong you can read your own 120 lines
  of texgen code, rather than debugging someone else's emulation layer through a JS bridge.
- **The game is small enough to make this proportionate.** Thirty entry points, one primitive type,
  no lighting.

### 2.4 The frame-timing problem

Do not skip this — it is the difference between "it runs" and "it plays right."

The game is **fully frame-locked**. Every duration is `seconds × FRAME_RATE` frames, the timer is
`1000/80` = 12 ms (integer division — so 83.3 fps, not 80), and there is no accumulator
([FINDINGS #17](FINDINGS.md#17-frame_rate-timing-is-4-fast-)).

In a browser you get `requestAnimationFrame`, which is usually 60 Hz and sometimes 120 Hz or 144 Hz.
At 60 Hz **the game runs 26% slow**; on a 120 Hz display it runs 44% fast.

Fix it with a fixed-timestep accumulator, which preserves the existing frame-based logic exactly:

```c
static double accumulator = 0.0;
const double STEP = 1.0 / FRAME_RATE;

void frame(double dt) {
    accumulator += dt;
    while (accumulator >= STEP) { MainGameLoopStep(); accumulator -= STEP; }
    display();
}
```

This keeps every existing frame-counted constant valid and decouples rendering from simulation. It
is the right fix natively too.

Also restructure `glutMainLoop` → `emscripten_set_main_loop`. The browser cannot block on an
infinite loop; `-sASYNCIFY` can paper over this but costs size and speed, and the loop here is
simple enough that restructuring is easy.

### 2.5 Assets

Simplest path — no code change at all:

```sh
emcc ... --preload-file assets
```

This packs `assets/` into a `.data` file and mounts a virtual filesystem, so the existing
`fopen("assets/sky2.sgi", "rb")` in `OpenAsset()` works untouched. The 16 textures are ~2 MB of
`.sgi`, which is acceptable.

Better later: convert the `.sgi` files to PNG (~40% smaller, and the browser decodes them natively),
or to a compressed GPU format. Not worth doing until the rest works.

### 2.6 Input in the browser

- **Pointer lock is required.** `glutPassiveMotionFunc` deltas from absolute coordinates already
  break at the screen edge natively; in a canvas they are worse. Request `requestPointerLock` on
  first click and read `movementX`/`movementY`.
- **Keyboard.** Emscripten's GLUT maps browser key events to the GLUT callbacks. `W`/`A`/`S`/`D`/space
  work as-is; make sure to `preventDefault` on space so the page does not scroll.
- **The difficulty buttons need fixing.** They are hit-tested against hard-coded pixel coordinates
  assuming a 1450×820 window ([FINDINGS #15](FINDINGS.md#15-difficulty-buttons-hit-tested-against-hard-coded-pixels-)).
  A canvas can be any size, so these must become fractions of the current viewport.

### 2.7 Suggested order

1. **Spike Route A** (½ day) — one textured quad with `GL_OBJECT_LINEAR` texgen under
   `-sLEGACY_GL_EMULATION=1`. Decides A vs B. Do this first.
2. Fix the `Map[]` out-of-bounds write (5 min) and add texture objects (~2 h), natively, where you
   still have a debugger.
3. Build the shim natively against desktop GL — `GL_QUAD_STRIP` → `GL_TRIANGLE_STRIP` first, then
   matrices, then immediate mode, then texgen. Verify each stage against the working macOS build,
   which is your reference implementation.
4. Add the fixed-timestep accumulator.
5. Swap GLUT for GLFW natively; confirm Linux.
6. Only then compile to WASM. By this point the graphics work is done and you are debugging
   Emscripten plumbing, not rendering.

The critical discipline is step 3: **always have the working macOS build to diff against.** That is
the entire reason `original/` is kept byte-for-byte.

---

## Level 3 — Modernisation (optional)

Worth doing only if the project becomes something you want to keep working on. In rough
value-per-effort order:

1. **Split the unity build** into real headers and translation units, removing the
   `#ifndef OBJ` duplication. Mechanical but touches every file.
2. **References instead of by-value objects** in the collision and draw paths
   ([FINDINGS #13](FINDINGS.md#13-objects-passed-and-returned-by-value-every-frame--performance)).
3. **Vertex buffers instead of immediate mode.** The vertex tables in `GLConstants.cp` are already
   static arrays — they upload to a VBO almost verbatim.
4. **Cull the map draw.** Five full passes over 8,382 cells every frame with no culling
   ([ARCHITECTURE #8](ARCHITECTURE.md#the-draw-path)). Batching by texture object collapses it to
   one pass; a frustum check would cut it much further.
5. **A real level format.** Lift `OriginalMap[]` and the eight `case` labels in `CheckEvents()` out
   into a data file. This is what would make the game *editable* rather than merely runnable.

None of this is needed to ship a working browser build. Do not let it block Level 2.
