# 3D Maze Game – Computer Graphics Course Project

A complete, fully interactive 3D game built from scratch in **C++** using **OpenGL** (fixed-function pipeline with freeglut/GLUT).

---

## 🌟 Graphics Concepts Demonstrated

| Feature | Lecture Ref | Implementation Detail |
|---|---|---|
| **3D Transformations** | Lecture 006 | Hand-coded 4x4 matrix math (`graphics/Transform.cpp`) for Translation, Rotation (arbitrary axis via Rodrigues' formula), and Scaling using Homogeneous Coordinates ($T \cdot R \cdot S$). |
| **Camera System** | Lecture 001/002 | Perspective ($60^\circ$ FOV) & Orthographic projection modes (`graphics/Camera.cpp`). 3 Camera views: **First-Person** (head height look-dir), **Third-Person** (orbiting camera), and **Top-Down** (overhead orthographic view). |
| **Lighting Models** | Lecture 001 | Blinn-Phong Illumination model ($I = K_a I_a + K_d I_d (N \cdot L) + K_s I_s (N \cdot H)^n$). **2 Light sources**: Directional Sun (`GL_LIGHT0`) + Point torch light following player (`GL_LIGHT1`). |
| **3D Modeling** | Lecture 004/005 | Primitives: Cube, UV Sphere, Cylinder, Cone, Pyramid, Torus, Floor (`objects/Primitives.cpp`). Outward normal vectors set per vertex. |
| **Hierarchical Modeling** | Lecture 006 | Composite objects built with `glPushMatrix` / `glPopMatrix` parent-child trees: Player (Body $\rightarrow$ Eye $\rightarrow$ Pupil), NPC Guard (Body $\rightarrow$ Head $\rightarrow$ Eyes/Arms/Legs). |
| **Animation** | Lecture 001 | Delta-time frame-based movement. Path animation for patrolling NPC guards. Interactive spin/wobble for gems/goal and ball rolling animation. |
| **Interaction** | - | Camera-relative **WASD / Arrow Keys** movement (W always moves into the screen, A/D strafe), Mouse Drag for orbiting/looking, Mouse Wheel to zoom, Key `V` / `1-3` to switch camera mode, `L` to toggle lighting, `Space` for pause, `R` for restart. |

---

## 📁 Project Structure

```
MazeGame/
├── CMakeLists.txt
├── Makefile
├── README.md
├── main.cpp
├── graphics/
│   ├── Camera.h / Camera.cpp        # 1st person, 3rd person orbit, Top-down orthographic
│   ├── Light.h / Light.cpp          # Blinn-Phong directional & point lights + material presets
│   └── Transform.h / Transform.cpp  # 4x4 Homogeneous matrix transformations
├── objects/
│   ├── GameObject.h / GameObject.cpp # Base class with hierarchy & bounding sphere collision
│   ├── Primitives.h / Primitives.cpp # Cube, Sphere, Cylinder, Pyramid, Cone, Torus, Floor
│   └── ComplexObject.h / ComplexObject.cpp # Composite objects (Player, Guard, Gem, Star, Wall)
└── game/
    ├── Game.h / Game.cpp            # Top-level state machine, loop, HUD, GLUT callbacks
    ├── Player.h / Player.cpp        # Movement, axis-aligned wall sliding, ball roll physics
    └── Scene.h / Scene.cpp          # 15x15 Maze layout, Skybox, Gem/Guard placement & updates
```

---

## 🛠️ Build & Run Instructions

### 1. Windows (MinGW / MSYS2 / GCC)
Make sure `freeglut` development libraries are installed.
```bash
g++ -o MazeGame main.cpp graphics/*.cpp objects/*.cpp game/*.cpp -lfreeglut -lglu32 -lopengl32 -lm
./MazeGame
```

### 2. Linux (Ubuntu / Debian / Fedora)
Install required packages:
```bash
sudo apt-get install build-essential freeglut3-dev libglu1-mesa-dev
```
Build using Makefile:
```bash
make
./MazeGame
```

### 3. CMake (Cross-Platform)
```bash
mkdir build
cd build
cmake ..
cmake --build .
./MazeGame
```

---

## 🎮 Game Controls

- **W / A / S / D** or **Arrow Keys**: Move the ball **relative to the camera** (W = into the screen, S = back, A/D = strafe left/right). The ball automatically faces the direction it rolls.
- **Q / E**: Turn the ball in place (affects which way it faces)
- **Mouse Drag**: Look around in 1st Person / Orbit camera around player in 3rd Person (release to auto-follow behind the player)
- **Mouse Wheel**: Zoom camera in / out (3rd Person)
- **1**: Switch to 1st Person View
- **2**: Switch to 3rd Person View
- **3**: Switch to Top-Down Orthographic View
- **V**: Cycle through Camera Modes (1st $\rightarrow$ 3rd $\rightarrow$ Top-Down)
- **L**: Toggle Directional Sun Light
- **Space**: Pause / Resume Game
- **R**: Reset / Restart Game
- **ESC**: Exit Application
