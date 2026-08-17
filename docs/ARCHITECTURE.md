# Architecture — how the 2007 tank game actually works

This is a reconstruction from the source, cross-checked against Ed Martin's own
[`original/Readme.txt`](../original/Readme.txt). Where his description and the code disagree, the
code wins and I say so.

---

## 1. The big structural decision: it is one translation unit

This is the first thing to understand, because everything else follows from it.

There are 13 `.cp` files, but **only one is ever compiled**. `tankmain.cp` `#include`s the other
twelve directly:

```c
#include "GameConstants.cp"
#include <GLUT/glut.h>          // → now "Platform.h"
#include "GLConstants.cp"
#include "GLGameObjectDrawFunctions.cp"
#include "GLTextures.cp"
#include "TankClass.cp"
#include "TurretClass.cp"
#include "TubeClass.cp"
#include "BulletClass.cp"
#include "Control.cp"
#include "GLGeneral.cp"
#include "MainGameRoutines.cp"
```

So the build is literally:

```sh
c++ -std=c++03 src/tankmain.cp -o tank -framework GLUT -framework OpenGL
```

There is no linking step to get wrong, no header/implementation split, no build system. This is why
the port was easy — there is no configuration surface to be stale.

### The include-guard trick, and why it is fragile

Several files need the same declarations, so the author guarded blocks with `#ifndef OBJ`,
`#ifndef GL`, `#ifndef CONTROL`, `#ifndef GLGENERAL`. `GLGeneral.cp` and `Control.cp` *each* contain
a full copy of the `#ifndef OBJ` block that declares the four global game-object arrays:

```c
#ifndef OBJ
	#define OBJ
	#include "TankClass.cp"
	...
	TankClass	Player1(0,0);
	TurretClass Turret[MAX_TURRETS];
	TubeClass	Tube[2];
	BulletClass Bullet[MAX_BULLETS];
#endif
```

`tankmain.cp` defines `OBJ` before it includes either of them, so only *its* copy is expanded and
the duplicates are inert. **It works, but it depends entirely on include order.** Reordering the
includes in `tankmain.cp`, or compiling any other `.cp` file directly, produces either duplicate
definitions or missing ones. Treat `tankmain.cp`'s include order as load-bearing.

---

## 2. Startup

`main()` in `tankmain.cp` does four things:

1. `RunMode = STARTSCREEN`
2. `OpenGLInit(argc, argv)` — creates the GLUT window, registers callbacks, loads all 16 textures
3. `GameInit()` — resets objects, copies `OriginalMap[]` → `Map[]`, applies difficulty
4. `glutTimerFunc(1000/FRAME_RATE, MainGameLoop, 1)` then `glutMainLoop()`

Note step 4: **the game does not drive itself from the GLUT display callback.** It registers
`display` with `glutDisplayFunc`, but then never calls `glutPostRedisplay()`. Instead
`MainGameLoop` calls `display()` *directly* at the end of each tick and re-arms the timer. The
`glutDisplayFunc` registration only matters for the initial paint and window damage events.

`FRAME_RATE` is 80, so the timer interval is `1000/80` — **integer division, so 12 ms, giving a
target of 83.3 fps, not 80.** Every duration constant in the game is expressed in frames as
`seconds * FRAME_RATE`, so all in-game timings run about 4% fast. Harmless, but it means the
"timers are based on a per second basis" claim in the original Readme is slightly off.

---

## 3. The four run modes

`RunMode` is a global int and is the game's entire top-level state machine. `MainGameLoop` branches
on it:

| Mode | What happens |
|---|---|
| `STARTSCREEN` | Camera parked facing a texture mounted on a wall. Animation timers tick, scene draws. Mouse clicks pick difficulty. |
| `TRANSITION` | Camera flies from the start screen wall to the gameplay start position. |
| `TUBE` | Boss approach. Camera circles the player; the player is frozen. Bullets and the Tube update; the player only updates if the train would run them over. |
| `NORMAL` | Full gameplay loop. |

Plus three debug/edge modes: `TOP_VIEW` (overhead camera), `PAUSED`, and `END` (boss dead — the
camera rotates freely forever).

### The normal loop, in order

```
CheckEvents()                     process the event queue
update animation timers           ObjectSpin, CameraShakeTime
Player1.Update()                  movement, firing, health, death
Turret[t].Update()   × 20         aim, fire, explode
Tube[t].Update()     × 2          boss state machine
Bullet[b].Update()   × 400        ballistics
CheckPlayerMapCollisions()
CheckBulletMapCollisions()
CheckBulletPlayerCollisions()
CheckBulletEnemyCollisions()      × 20
CheckBulletTubeCollisions()       × 2
positionCamera(Player1)
display()
```

---

## 4. The world: a text map

`GLConstants.cp` holds the level as a literal `const int OriginalMap[66 * 127]` array — 8,382
cells, written out as readable rows of single letters:

```c
0,0,0,B,T,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,...
0,0,0,B,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,8,X,X,X,...
```

The letters are `const int` aliases declared in `GameConstants.cp`, which is a genuinely elegant
trick — the map is legible as ASCII art but compiles to a plain int array:

| Letter | Constant | Meaning |
|---|---|---|
| `0` | — | void / nothing drawn |
| `X` | `GROUND` (20) | floor |
| `B` | `WALL` (21) | wall block |
| `A` | `ARCH` (22) | archway |
| `T` | `TOWER` (23) | tower block |
| `I` | `RAIL` (24) | train rail |
| `H` | `RAILSLEEPER` (25) | rail sleeper |
| `R` | `RAPID_FIRE` (27) | rapid-fire pickup |
| `D` | `DUAL_FIRE` (28) | dual-cannon pickup |
| `M` | `MED_PACK` (29) | health pickup |
| `2`–`9` | — | **one-shot ambush triggers** |

Indexing is `Map[x + MAP_X*z]` with `MAP_X = 66`, `MAP_Z = 127`. World coordinates are
`map coordinate × MAP_BLOCK_SIZE` where `MAP_BLOCK_SIZE = 1.3`. The Y axis is up; the map is the XZ
plane.

`Map[]` is the mutable working copy; `OriginalMap[]` is the pristine one, re-copied by `GameInit()`
on every restart. Triggers are erased from `Map[]` when they fire, which is how they stay one-shot.

### Trigger cells 2–9

Driving over a numeric cell fires a `NEW_MAP_EVENT`. `CheckEvents()` erases every cell with that
number from the map, then spawns a scripted turret ambush at hard-coded coordinates. Cell `9` is
special — it wakes the boss:

```c
case 9:
    RunMode = TUBE;
    CameraShakeTime = 2*MAX_CAMERA_SHAKE_TIME;
    Tube[0].InitTube(MAP_BLOCK_SIZE * 41.5, 0, MAP_BLOCK_SIZE * 101.5);
    Tube[1].InitTube(MAP_BLOCK_SIZE * 60.5, 0, MAP_BLOCK_SIZE * 101.5);
```

So the entire level script — every ambush in the game — is those eight `case` labels in
`MainGameRoutines.cp`. There is no level data format beyond the map array.

---

## 5. The event system

Deliberately primitive, and worth understanding because it is the only inter-object communication
channel in the game.

```c
float events[10 * MAX_EVENTS];        // MAX_EVENTS = 100
void Event(int description, float a, float b, ... float i);
```

A flat `float` array of 100 slots × 10 fields. `Event()` linear-scans for the first slot whose
field 0 is zero and writes there. `CheckEvents()` walks the array each frame and dispatches on
field 0:

- `NEW_BULLET_EVENT` — spawn a bullet with the given position/velocity/type
- `NEW_TURRET_EVENT` — spawn a turret
- `NEW_MAP_EVENT` — a map trigger fired

Everything is a `float`, including the discriminant and integer map indices, which are cast back
with `int(events[e*10 + 1])`. It works because the values are small, but it is why you see
`int(...)` casts everywhere in the dispatcher.

---

## 6. The four game object classes

All four follow an identical shape — private state, `Set*`/`Get*` accessors, `Update()`, `Draw*()`.
None of them use inheritance or virtual functions; there is no common base class.

| Class | Instances | Role |
|---|---|---|
| `TankClass` | `Player1` | The player. Position, velocity, independent turret and track directions, health, lives, cannon type, key/mouse state. |
| `TurretClass` | `Turret[20]` | Wall-mounted enemy turrets. Aim at the player, fire on a cooldown, explode. |
| `TubeClass` | `Tube[2]` | The boss — two halves of an armoured train. Six-state machine (see below). |
| `BulletClass` | `Bullet[400]` | Projectiles: player, enemy, fire, shrapnel. |

All four are **fixed-size global arrays**, not pointers and not dynamically allocated. The author
notes in the Readme that he tried pointers and hit memory bugs; the commented-out pointer version is
still in `tankmain.cp`. Given the rest of the code, the fixed arrays were the right call.

### The Tube boss state machine

`TubeClass::Update()` cycles through:

```
TUBE_STATE_APROACHING     → train rolls in, camera circles player, player disabled
TUBE_STATE_NEW_TURRETS    → spawns turrets + drops a random pickup
TUBE_STATE_AIMING         → aims at player
TUBE_STATE_OPENING_DOORS  → doors and windows animate open
TUBE_STATE_OPEN_FIRE      → fires, and is vulnerable through the windows
TUBE_STATE_CLOSING_DOORS  → closes, cycle repeats
```

Each state has a duration constant (`TUBE_*_TIME`) scaled by difficulty. The boss is only damageable
during `OPEN_FIRE`, and only by bullets that pass through one of the three window rectangles — see
`CheckBulletTubeCollisions` in `Collisions.cp`.

---

## 7. Collision detection

`Collisions.cp` — five functions, all analytic, no broad phase, no spatial structure.

**Player vs map.** Trig gives the tank's four corners; sample points are spread along the front and
back bumpers. Each sample is converted to a map cell and compared against **only the 25 surrounding
cells** (a 5×5 window) rather than the whole map. On a wall hit: restore the previous position,
reverse the velocity, and push back along the collision normal.

**Bullet vs map.** Each bullet is swept: 10 sample points are interpolated between the previous and
current position using the velocity vector, so fast bullets do not tunnel through walls. Each sample
maps to a cell and dispatches on cell type.

**Bullet vs player.** The clever one. Rather than rotating the tank's bounding box, it transforms the
*bullet* into the tank's local frame: compute distance and angle between the two, then use trig to
get `AuxXSQUARED` (how far in front) and `AuxZSQUARED` (how far to the side). A hit is then a
trivial axis-aligned comparison against the tank's length and width.

**Bullet vs turret.** Same 10-point sweep; turrets are treated as spheres, so it is a distance
check against radius sum.

**Bullet vs tube.** Sweep, then check the bullet is over a `RAILSLEEPER` cell, within the train's
length, and below its height — plus the separate window check for actual damage.

Note that every one of these functions takes and returns objects **by value**:

```c
Player1 = CheckBulletPlayerCollisions(Player1, Bullet);
Turret[t] = CheckBulletEnemyCollisions(Turret[t], Bullet);
```

So each object is deep-copied twice per collision call, every frame. Correctness is fine; it is
purely a performance matter.

---

## 8. Rendering

This is the part that matters most for any port, so it is worth being precise about what the game
actually uses.

**The complete GL feature list:**

```
 154 × glVertex3f          73 × glBegin/glEnd
  65 × glColor3f           50 × glRotated
  38 × glTranslated        24 × glTranslatef
  19 × glTexGeni           15 × glTexImage2D
   7 × glutSolidSphere      5 × gluLookAt
   1 × gluPerspective
```

Primitive modes: **69 × `GL_QUAD_STRIP`, 4 × `GL_POLYGON`.** Nothing else.

Three observations shape everything downstream:

**a) There is no lighting.** Not one `glLight*`, `glMaterial*` or `glNormal3f` call in the game
proper. Shading is flat `glColor3f` plus texture. This is enormously good news for porting —
fixed-function lighting is normally the hardest part of a GL 1.x migration, and here there is none
to migrate. Depth ("this face is darker") is baked into the hand-picked vertex colours.

**b) Geometry is hand-authored vertex tables.** `GLConstants.cp` is ~490 lines of `const GLfloat`
arrays: turret barrels, tank tracks, train doors, all typed out as coordinates. Draw functions in
`GLGameObjectDrawFunctions.cp` walk those tables inside `glBegin(GL_QUAD_STRIP)` loops. There are no
model files and no mesh loading.

**c) There are no texture objects.** This is the big one. Search the source for `glGenTextures` or
`glBindTexture` and you get **zero hits**. Instead, every draw call that needs a different texture
calls `glTexImage2D` and re-uploads the entire 256×256 RGBA image:

```c
glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, sand1);
```

There are 15 such call sites, most inside the per-frame draw path. That is **15 × 256 × 256 × 4 =
~3.9 MB of texture upload per frame**, or roughly 315 MB/s at the target frame rate, purely to
switch between 16 static images that never change. On 2007 hardware with a fat AGP/PCIe bus this was
survivable. It is the first thing to fix in any port, and it is *mandatory* before WebGL.

### Texture coordinate generation

The game never emits a single `glTexCoord2f`. All texture coordinates come from `glTexGeni`:

| Mode | Uses | Where |
|---|---|---|
| `GL_OBJECT_LINEAR` | 11 | Ground, walls, towers — texture projected from object space, so it tiles with world position |
| `GL_EYE_LINEAR` | 2 | Turrets |
| `GL_SPHERE_MAP` | 6 | The Tube boss — a fake chrome/environment reflection |

This is the single most awkward thing to port, because none of it exists in WebGL or GLES2. It is
covered in detail in [`PORTING.md`](PORTING.md), including why it turns out to be more tractable
than it first looks.

### The draw path

`display()` in `GLGeneral.cp`:

1. `glClear`
2. `drawmap(...)` — the whole map, every frame
3. `Player1.DrawTank()`
4. all 400 bullets (`DrawBullet` early-outs on inactive)
5. all 20 turrets, with `GL_EYE_LINEAR` texgen
6. both tubes, with `GL_SPHERE_MAP` texgen
7. `glutSwapBuffers()`

`drawmap()` makes **five full passes over all 8,382 cells** — one for ground, one for walls, one for
towers, and one combined switch for arches/rails/pickups — because each pass needs a different
texture bound (which, per above, means a different full re-upload). That is ~42,000 loop iterations
per frame with no culling of any kind. The author flags this himself: *"there's room for
optimisation here, although arguably unnecessary in this scale."* He was right for 2007; batching
by texture object would collapse it to one pass.

### The camera

`positionCamera()` is a chase camera with two independent smoothing terms:

```c
CurrentCameraDirectionRightLeft = 180 - Player1.GetturretRightLeft();   // follows the turret, not the hull
CurrentCameraDistance -= (CurrentCameraDistance - ObjectiveCameraDistance)/80;   // ease distance
camera_x += -(camera_x - new_camera_x) / 1.9;                                    // ease position
```

The camera follows the **turret** direction rather than the hull, which is what makes aiming feel
like a twin-stick shooter rather than a driving game. `ObjectiveCameraDistance` is nudged by map
triggers (`case 8` pulls it in by 10%, `case 2` pushes it back out), so the framing tightens in
corridors. Camera shake is a `pow(t/T, 2)` falloff applied as random jitter on all three axes.

Both smoothing terms are **per-frame, not per-second** — they assume a fixed timestep. If you ever
decouple simulation from rendering, these need reworking or the camera feel changes.

---

## 9. Textures and the SGI loader

`GLTextures.cp` is two things bolted together: a Silicon Graphics `.sgi`/`.rgb` reader lifted from
the Xcode sample projects, and Ed Martin's loader on top.

All 17 shipped images are 256×256, magic `0x01DA`, RLE-compressed (`type & 0xFF00 == 0x0100`), with
3 or 4 channels. The reader handles both RLE and raw scanlines, does runtime endian detection, and
expands everything to RGBA.

Two notes on the loader:

- The header is read with `fread(image, 1, 12, image->file)` **directly into the `ImageRec`
  struct**. Only the first 12 bytes are used, so the struct's later `unsigned long` fields (which
  differ in size between LP64 and Windows) do not affect correctness — but it is relying on the
  compiler not inserting padding in the first 12 bytes.
- `ImageGetRow()` RLE-decodes into `buf` with **no bounds checking**. A malformed `.sgi` can
  overflow the destination. Fine for shipped assets, not fine if you ever load untrusted images.

**The archive is incomplete.** The loader asks for 16 textures; `sky1.sgi` is missing. Two files,
`ed4.sgi` and `sand2.sgi`, are present but never loaded — `ed4.sgi` is the only 3-channel outlier
and looks like a leftover. `steel1.tiff` and `wall1.tiff` are source art, not used at runtime.

---

## 10. Input

`Control.cp`, four GLUT callbacks. Keyboard down/up set boolean flags on `Player1`; the actual
movement happens in `TankClass::Update()`, which is the right structure.

Mouse aiming uses `glutPassiveMotionFunc` and computes a delta against static `oldx`/`oldy`:

```c
void InGameMouseMove(int x, int y) {
    static int oldx, oldy;
    Player1.SetMousemove(oldx-x, oldy-y);
    oldx = x; oldy = y;
}
```

Because these are absolute window coordinates with no pointer capture, **the turret stops turning
once the cursor hits the edge of the screen** — the author calls this out in his Readme as a known
problem. The fix on any platform is pointer lock / relative mouse mode, which is also exactly what
a browser port needs (`requestPointerLock`).

`InGameMouseButtons` is start-screen only, and hit-tests difficulty buttons against **hard-coded
pixel coordinates** (`y < 524 && y > 410`, `x < 572 && x > 360`, …) that assume the default
1450×820 window. Resize the window and the buttons are in the wrong place.
