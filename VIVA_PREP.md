# VIVA / Discussion Prep Guide — 3D Maze Game

Study guide for the Computer Graphics presentation & viva.
Covers every grading criterion with exact code locations.

---

## 1. TRANSFORMATIONS — "Explain your matrix class"

**Say:** "I use 4x4 homogeneous matrices. A point (x,y,z) becomes (x,y,z,1), so translation becomes a matrix multiply. OpenGL stores them column-major, so translation sits in m[12], m[13], m[14]."

| Question | Answer | Proof in code |
|---|---|---|
| Where is translation? | `Mat4::translation` | `graphics/Transform.cpp:25-32` (column 3 = tx,ty,tz) |
| Where is rotation? | `rotationX/Y/Z` (`Transform.cpp:35-65`), arbitrary axis via Rodrigues' formula (`:73-89`) | cos/sin placed by axis |
| Where is scaling? | `Mat4::scaling` | `Transform.cpp:92-98` (diagonal only) |
| Why does order matter? | Matrices don't commute. `T*R` = rotate first then move; `R*T` moves then rotates around a different point | `game/Player.cpp:draw()` composes `Mat4::translation(pos) * Mat4::rotationY(yaw)` |
| Show me live scaling | Gem/star breathing pulse via `glScalef` | `objects/ComplexObject.cpp:40-44, 203-209` |
| What is the matrix stack? | `glPushMatrix` saves, `glPopMatrix` restores — used for parent->child hierarchies | `objects/ComplexObject.cpp` everywhere |

---

## 2. CAMERA — "Explain your camera system"

**Say:** "I have 3 modes. First/third person use perspective projection via `gluPerspective` (60° FOV) — things shrink with distance. Top-down uses `glOrtho` — parallel rays, no shrinking. The view transform is `gluLookAt(eye, center, up)` which builds the lookAt matrix from forward = center-eye, right = forward x up, true-up = right x forward."

| Question | Answer | Proof |
|---|---|---|
| Difference between the two projections? | Perspective = pinhole model, foreshortening; Orthographic = box mapping, no depth shrinking | `graphics/Camera.cpp:192-215` |
| Where's the view matrix? | `gluLookAt` | `Camera.cpp:221-227` `applyView()` |
| How does the 3rd-person orbit work? | Orbit yaw/pitch + distance -> `eye = player + (dist*cos(pitch)*sin(yaw), dist*sin(pitch), ...)` | `Camera.cpp:128-138` |
| Why `up = (0,0,-1)` in top-down? | So screen-top = world "north"; with up=(0,1,0) the view would be upside-down | `Camera.cpp:146` |
| Why smooth follow? | Exponential lerp `t = min(1, smoothSpeed*dt)` prevents camera teleport jumps | `Camera.cpp:171-184` |
| Why `resolveCameraEye`? | Prevents the camera clipping through maze walls (sampled along the line) | `game/Scene.cpp:213-249` |

---

## 3. LIGHTING — "Explain your lighting model" (MOST IMPORTANT)

**Memorize the Blinn-Phong equation:**

> **I = Ka*Ia + Kd*Id*max(0, N·L) + Ks*Is*max(0, N·H)^n**
> ambient + diffuse (Lambert) + specular, where H = normalize(L+V)

| Question | Answer | Proof |
|---|---|---|
| What is ambient? | Base illumination so dark sides aren't pure black; direction-independent | `graphics/Light.h:10-19`; global ambient `game/Game.cpp:100-101` |
| Diffuse? | Lambert's law — brightness proportional to N·L; max(0,...) because light can't hit a back face | `Light.h:13` |
| Specular? | Highlight proportional to (N·H)^n; n = shininess (higher = tighter spot) | `Light.h:14,17` |
| Directional vs point light? | The **w component**: w=0 -> direction only (infinite distance, sun); w=1 -> actual position (torch) | `graphics/Light.cpp:63-65` |
| What is attenuation? | Point light fades: 1/(kc + kl*d + kq*d^2) | `Light.h:62`, `Light.cpp:67-72` |
| What's a material? | Ka, Kd, Ks, n — how the *surface* reflects, vs the light's colors | 8 presets `Light.cpp:88-151` (Gold 51.2, Ruby 76.8, ...) |
| Why `glEnable(GL_NORMALIZE)`? | After scaling, normals are no longer unit length -> normalized on GPU so lighting is correct | `Game.cpp:83` |
| Why are normals important? | N·L needs unit normals; they must point OUTWARD | `objects/Primitives.cpp:8-12` |

---

## 4. MODELING — "How did you build the objects?"

**Say:** "I hand-wrote every primitive — no glutSolidCube. Each vertex has an outward normal so lighting shades it correctly. Complex objects are hierarchies: the player ball is body -> eye -> pupil; a guard is body -> head -> eyes -> arms/legs. Each part gets its own push/translate/rotate/pop block, so children inherit parent transforms."

| Question | Answer | Proof |
|---|---|---|
| How do you compute sphere normals? | N = position/radius (radial) | `Primitives.cpp:71-84` |
| How does the torus work? | Parametric: x=(R+r·cosφ)cosθ, y=(R+r·cosφ)sinθ, z=r·sinφ | `Primitives.cpp:209-242` |
| The 3-level hierarchy? | body -> eye -> pupil, nested push/pop | `ComplexObject.cpp:75-108` |
| Guard walk animation? | swing = sin(animTime·5)·25°; arms and opposite legs swing in phase | `ComplexObject.cpp:124-178` |
| How are walls placed? | 15x15 grid array; `cellCenter(row,col)` converts grid -> world | `Scene.cpp:38-54, 85-91, 365-377` |

---

## 5. ANIMATION — "Why is movement smooth on different computers?"

**Say:** "I use delta-time (frame-rate independent) animation: position += velocity·dt. Whether the PC runs 30 or 144 FPS, the ball moves at the same real-world speed. `Game::idle` computes dt from `glutGet(GLUT_ELAPSED_TIME)` and clamps it to 0.1s to avoid jumps when dragging the window."

| Question | Answer | Proof |
|---|---|---|
| Ball roll math? | Arc length = radius × angle -> angle = distance/radius | `game/Player.cpp:103-109` |
| Exponential easing? | k = 1 - e^(-9·dt) gives natural acceleration, frame-rate independent | `Player.cpp:92-97` |
| Guard patrol? | Linear interpolation between waypoints with wall sliding (X then Z tested separately) | `Scene.cpp:181-209` |
| What drives the loop? | GLUT idle callback -> update -> glutPostRedisplay | `main.cpp:29-31`, `game/Game.cpp:139-173` |

---

## 6. INTERACTION — "How does input work?"

**Say:** "GLUT is event-driven — I register callbacks (glutKeyboardFunc, glutMouseFunc...). Keys set flags on press, clear on release; the update loop reads the flags each frame. Mouse drag in 3rd person changes orbit yaw/pitch; in 1st person horizontal drag turns the player so WASD moves where you look."

Callbacks registered at `main.cpp:82-93`.

---

## TRAP QUESTIONS TO PREPARE

1. "Why is your eye off-center?" -> The pupil is a child of the eye (`ComplexObject.cpp:99-103`) — hierarchy demo, not a bug.
2. "What happens if you scale a light?" -> Nothing — lights are positional/directional, they have no size. (Related: why `GL_NORMALIZE`.)
3. "Why two lights and not one?" -> Sun gives global shape; torch gives warm localized light that follows the player (`Game.cpp:107-122`).
4. "Difference between glRotatef before vs after glTranslatef?" -> Order matters: T·R = place then spin in place; R·T = orbit around origin.
5. "Is the timer still running when paused?" -> No — the whole update block is skipped when state != PLAYING (`Game.cpp:147`).
6. "What is w in the position array?" -> Homogeneous coordinate; 0 = direction (point at infinity), 1 = position. THE classic CG question.

---

## 45-SECOND ELEVATOR SUMMARY (memorize this)

> "My project is a 3D maze game in C++/OpenGL. I wrote my own 4x4 matrix math for translation, rotation and scaling in homogeneous coordinates. The camera has 3 modes — first person, third person orbit, and an orthographic top-down — using gluLookAt with both gluPerspective and glOrtho. Lighting is the Blinn-Phong model with a directional sun and a point torch that follows the player, with attenuation and 8 predefined materials. All objects are hand-built primitives with proper normals, composed hierarchically with the matrix stack — like the player ball with its eye and pupil. Animation is frame-rate independent using delta time: ball rolling by arc length, patrolling guards with swinging limbs, spinning gems and pulsing goal star. Interaction covers keyboard movement, mouse orbit, wheel zoom, three camera keys, light and axes toggles, pause and reset."