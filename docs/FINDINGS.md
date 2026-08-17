# Findings — bugs, hazards and landmines

Everything here was found while porting. Each entry says whether it is **fixed in `src/`** or **left
alone**. Nothing in `original/` was touched.

The author's Readme ends with *"That said, the current version is bug free."* It is remarkably close
to true for a two-month solo project — the game plays through without crashing. But there is one
genuine memory-safety bug, and several things that only worked by accident.

---

## Fixed in `src/`

### 1. C++11 narrowing errors — ~200 hard errors 🔴 blocker
**`GLConstants.cp`, throughout** · *fixed via build flag*

```c
const GLfloat TurretBarrel[] = { -0.6 * ENEMY_TURRET_SIZE, 0.6 * ENEMY_TURRET_SIZE, 0, ... };
```

`ENEMY_TURRET_SIZE` is a `const float`, so `0.6 * ENEMY_TURRET_SIZE` promotes to `double`. C++11
forbids narrowing `double` → `float` in a braced initialiser unless the value is a constant
expression the compiler can prove fits. These are not constant expressions (they involve a non-`constexpr`
`const float`), so every one is an error under C++11 and later.

**Fix:** compile with `-std=c++03`. No source change.
**Alternative if you ever need a modern dialect:** make the size constants `constexpr`, or wrap each
value in `static_cast<GLfloat>(...)`. The former is one line, the latter is ~200 edits.

---

### 2. Missing texture kills the process before the window opens 🔴 blocker
**`GLTextures.cp` · `create_textures()`** · *fixed*

The loader requests 16 textures. `sky1.sgi` **is not in the archive** — it was lost somewhere between
2007 and now. The original code called `exit(1)` on any missing file, so the game died at startup
with `sky1.sgi: No such file or directory` before creating a window.

`sky1` is genuinely used — it is the sky dome texture in `DrawSky()`
(`GLGameObjectDrawFunctions.cp:116`). `sky2` is a different texture used as the turret environment map.

**Fix:** `ImageOpen()` now returns `NULL` instead of `exit(1)`, and `create_textures()` was rewritten
from 16 copy-pasted blocks into a table with a `LoadTexture(name, comps, substitute)` helper. `sky1`
falls back to `sky2`; anything else missing falls back to a generated magenta/black checkerboard, so
an incomplete asset set gives you obviously-wrong textures rather than a dead process.

**Note:** the sky is therefore not what Ed Martin intended. If `sky1.sgi` ever turns up, drop it in
`assets/` and it will be picked up automatically.

---

### 3. `<Carbon/Carbon.h>` 🟡
**`GLConstants.cp:11`, `GLGeneral.cp:17`** · *fixed*

Included in two files, referenced in neither. Carbon is a deprecated macOS-only API. Removed — this
was pure dead weight blocking non-Apple builds.

---

### 4. Textures only loadable from one working directory 🟡
**`GLTextures.cp`** · *fixed*

The 2007 code `fopen()`ed bare filenames like `"sand1.sgi"`, so the binary only worked when run with
the CWD set to the texture folder. Added `OpenAsset()`, which tries the bare name first (preserving
old behaviour), then `$TANK_ASSETS/`, then `assets/`, then `../assets/`.

---

### 5. Aspect ratio hard-wired to 1.0 🟠 visible bug
**`GLGeneral.cp:201` · `reshape()`** · *fixed*

```c
gluPerspective(90, (GLfloat)height / (GLfloat)height, 2.0, 900.0);
//                          ^^^^^^              ^^^^^^  height/height == 1.0
```

Should be `width / height`. The scene was horizontally squashed in **any** non-square window,
including the shipped 1450×820 default. This is a plain typo and had been there since 2007.

**Fix:** `(GLfloat)width / (GLfloat)(height ? height : 1)` — the guard avoids a divide-by-zero when
a window is minimised to zero height, which some window managers do.

**Caveat:** this visibly changes the framing. Any screenshots or videos Ed Martin made in 2007 were
made with the squashed projection.

---

### 6. `glutInitDisplayMode` called after `glutCreateWindow` 🟠
**`tankmain.cp` · `OpenGLInit()`** · *fixed*

```c
glutCreateWindow("tank");
glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);   // too late
```

`glutInitDisplayMode` only affects windows created *after* it. As written, the window was created
with GLUT's default framebuffer and the requested depth and double-buffer bits were silently
ignored. It worked only because that default already includes both — which is not guaranteed across
GLUT implementations, and notably differs in freeglut and in Emscripten's GLUT.

**Fix:** moved before `glutCreateWindow`. This one matters specifically for the Linux and browser
ports.

---

### 7. `glEnable(GL_SHADE_MODEL)` is not a valid call 🟢 cosmetic
**`tankmain.cp` · `OpenGLInit()`** · *fixed*

`GL_SHADE_MODEL` is a `glGet` query token, not an enable capability. The call raised
`GL_INVALID_ENUM` and did nothing. The intent was smooth shading, which is the OpenGL default
anyway, so this is a no-op in practice — but it left a permanent error in the GL error queue, which
is annoying when debugging.

**Fix:** replaced with `glShadeModel(GL_SMOOTH)`.

---

## Left alone — deliberate

These are real, but fixing them means changing behaviour or doing a refactor. They are documented
rather than patched, so the port stays reviewable.

### 8. Out-of-bounds write to `Map[]` 🔴 **fix this first**
**`MainGameRoutines.cp:151` · `CheckEvents()`**

```c
for (x = 0; x <= MAP_X; x++)  for (z = 0; z <= MAP_Z; z++)
    if (Map[x + MAP_X*z] == int(events[e*10 + 1])) Map[x + MAP_X*z] = GROUND;
//         ^^ <= on both loops
```

`Map` is `int Map[66 * 127]` = **8,382** elements. With `<=` on both bounds the maximum computed
index is `66 + 66*127` = **8,448** — so this reads, and conditionally writes, up to **66 ints (264
bytes) past the end of the array**.

This is not theoretical. It runs every time a map trigger fires, which is every ambush in the game.
It has not caused a visible crash because `Map` is a global and whatever follows it in BSS absorbs
the damage — but it is undefined behaviour, and it is exactly the kind of thing that turns into a
hard crash under a different compiler, a different memory layout, or Emscripten's bounds-checked
heap.

**Fix:** change both `<=` to `<`. One character each.

**Why I left it:** it changes game behaviour in principle (the last row/column of triggers would no
longer be scanned — though in practice the map's edges are all `0`), and a behaviour change should
be your call, not mine. It is a two-character edit whenever you want it.

---

### 9. `GameInit()` clears only 8 of 100 event slots 🟠
**`MainGameRoutines.cp:366`**

```c
for (i = 0; i<8; i++)  for (j = 0; j<10; j++)  events[i*10 + j] = 0;
```

`MAX_EVENTS` is 100. Only the first 8 slots are cleared on restart, so `events[80..999]` keep
whatever was in them. It works on a **fresh start** because `events[]` is a global `float` array and
therefore zero-initialised — but after a death-and-restart, stale events queued beyond slot 8 will
be re-processed. Given `Event()` fills slots in order and the queue is drained every frame, going
past slot 8 requires a heavy frame; it is a latent bug, not a constant one.

**Fix:** `for (i = 0; i < MAX_EVENTS; i++)`.

---

### 10. Missing `break` — waking the boss also grants rapid fire 🟡
**`MainGameRoutines.cp` · `CheckEvents()`**

```c
case 9:
    RunMode = TUBE;
    Tube[0].InitTube(...);
    Tube[1].InitTube(...);
    // ← no break

case RAPID_FIRE:
    Player1.SetType(RAPID_FIRE);
    break;
```

`case 9` falls through into `case RAPID_FIRE`, so triggering the boss silently upgrades the player's
cannon. This may well be intentional generosity before the boss fight — but if so it is undocumented
and indistinguishable from a missing `break`. Worth a decision either way.

---

### 11. No texture objects — ~3.9 MB re-uploaded per frame 🟠 performance
**15 call sites across `GLGeneral.cp`, `GLGameObjectDrawFunctions.cp`, `TankClass.cp`**

Zero occurrences of `glGenTextures` or `glBindTexture` in the entire codebase. Every texture change
re-uploads a full 256×256 RGBA image with `glTexImage2D`:

```
15 uploads × 256 × 256 × 4 bytes = 3.93 MB per frame
                       at ~83 fps ≈ 326 MB/s
```

…to switch between 16 images that never change after load.

**Fix:** one `glGenTextures(16, ids)` at startup, upload each image once, replace the 15
`glTexImage2D` calls with `glBindTexture`. Perhaps 30 lines total.

**This is optional natively and mandatory for WebGL.** Browsers will not tolerate this upload rate.

---

### 12. Global `int y` doubles as the ground height 🟠 landmine
**`GameConstants.cp:135`, used in `GLGeneral.cp` · `drawmap()`**

`GameConstants.cp` declares `int y, iw, ih;` as texture-loading scratch variables. Then `drawmap()`
does:

```c
int x,z;                      // note: no y
...
DrawGround(x, y, z);          // ← uses the *global* y
```

The global `y` is never assigned, so it is 0, which happens to be the correct ground height. The
whole map renders at the right elevation **by accident**. If any texture code ever writes to `y`,
the floor moves.

**Fix:** pass an explicit `0`, or declare a local `const int y = 0`. Left alone because it is
currently harmless and touching it invites a silent visual change.

---

### 13. Objects passed and returned by value every frame 🟢 performance
**`MainGameRoutines.cp`, `Collisions.cp`, `GLGeneral.cp`**

```c
Player1   = CheckBulletPlayerCollisions(Player1, Bullet);
Turret[t] = CheckBulletEnemyCollisions(Turret[t], Bullet);
Tube[t]   = CheckBulletTubeCollisions(Bullet, Tube[t]);
```

Each call deep-copies the object in and out — twice per call, every frame, for 20 turrets and 2
tubes. `drawmap()` also takes `TankClass Player1` by value. Correct, just wasteful. References would
be a mechanical fix.

---

### 14. RLE decode has no bounds check 🟡 robustness
**`GLTextures.cp` · `ImageGetRow()`**

The RLE decoder writes into `buf` with no check against the destination size. A malformed or
hostile `.sgi` file can overflow it. Irrelevant for the 17 shipped assets; relevant the moment
anyone loads a texture they did not author. Inherited from the Silicon Graphics sample code, not
Ed Martin's.

---

### 15. Difficulty buttons hit-tested against hard-coded pixels 🟡
**`Control.cp` · `InGameMouseButtons()`**

```c
if ((y < 524) && (y > 410)) { if ((x < 572) && (x > 360)) ... }
```

Assumes the 1450×820 default window. Resize and the difficulty buttons are somewhere else. Needs to
be expressed as a fraction of the current window size — which is also required for a browser canvas
that can be any size.

---

### 16. Mouse aiming has no pointer capture 🟡
**`Control.cp` · `InGameMouseMove()`**

Deltas are computed from absolute window coordinates, so the turret stops turning when the cursor
reaches the screen edge. Ed Martin documents this himself. The fix is relative mouse mode / pointer
lock — see [`PORTING.md`](PORTING.md), where it is required anyway.

---

### 17. `FRAME_RATE` timing is 4% fast 🟢
**`tankmain.cp`, `MainGameRoutines.cp`**

```c
glutTimerFunc(1000/FRAME_RATE, MainGameLoop, 1);   // 1000/80 → integer division → 12 ms
```

12 ms is 83.3 fps, not 80. Since every duration in the game is `seconds * FRAME_RATE` frames,
everything runs ~4% fast. Also, the timer only bounds the *minimum* interval — there is no
accumulator, so on a slow frame the simulation silently slows down rather than catching up. The
game is fully frame-locked.

Relevant for the browser port, where you get whatever `requestAnimationFrame` gives you (usually
60 Hz, sometimes 120). At 60 Hz the game would run **26% slow**. Decoupling the timestep, or at
minimum scaling by measured delta time, is required for a browser build to feel right.

---

### 18. Include-order dependency 🟡 fragility
**`tankmain.cp`**

`GLGeneral.cp` and `Control.cp` each contain a complete copy of the `#ifndef OBJ` block that defines
the four global object arrays. `tankmain.cp` defines `OBJ` first, so the duplicates are inert.
Reorder the includes and you get duplicate-definition errors; compile any other `.cp` on its own and
you get a different set of failures. The include order in `tankmain.cp` is load-bearing and
undocumented. See [`ARCHITECTURE.md`](ARCHITECTURE.md#the-include-guard-trick-and-why-it-is-fragile).

---

## Summary

| Severity | Count | Status |
|---|---|---|
| 🔴 Blocker | 3 | 2 fixed, **1 open** (out-of-bounds `Map[]` write) |
| 🟠 Real bug | 6 | 3 fixed, 3 open |
| 🟡 Hazard | 6 | 2 fixed, 4 open |
| 🟢 Minor | 3 | 1 fixed, 2 open |

The build is green and the game runs. The one thing worth doing today, independent of any porting
plans, is the two-character fix to `MainGameRoutines.cp:151`.
