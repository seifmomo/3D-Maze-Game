#pragma once
/*
 * Game.h
 * ============================================================
 * Top-level game controller.
 * Owns all subsystems: Scene, Player, Camera, Lights.
 *
 * Responsibilities:
 *  - OpenGL initialisation (depth test, lighting, fog)
 *  - Frame loop (update → draw → swap buffers)
 *  - GLUT callback delegation (keyboard, mouse, reshape, idle)
 *  - Game state machine: PLAYING → WIN/LOSE/PAUSED
 *  - HUD overlay (score, timer, lives, camera mode indicator)
 *  - Win/lose screen with semi-transparent overlay
 * ============================================================
 */

#include "../graphics/Camera.h"
#include "../graphics/Light.h"
#include "Scene.h"
#include "Player.h"

enum class GameState { PLAYING, PAUSED, WIN, LOSE };

class Game {
public:
    Game();
    ~Game() {}

    // ── Lifecycle ─────────────────────────────────────────────
    void init(int windowW, int windowH);

    // ── GLUT callbacks ────────────────────────────────────────
    void display();
    void reshape(int w, int h);
    void keyDown   (unsigned char key, int x, int y);
    void keyUp     (unsigned char key, int x, int y);
    void specialDown(int key, int x, int y);
    void specialUp  (int key, int x, int y);
    void mouseButton(int button, int state, int x, int y);
    void mouseMotion(int x, int y);
    void idle();

private:
    Scene  scene_;
    Player player_;
    Camera camera_;
    Light  sunLight_;    // GL_LIGHT0 – directional (simulates sun)
    Light  pointLight_;  // GL_LIGHT1 – follows player (torch effect)

    GameState state_;
    float     timer_;       // seconds remaining
    float     totalTime_;
    int       score_;
    int       lives_;

    int   windowW_, windowH_;
    float lastFrameTime_;
    float deltaTime_;

    // Mouse drag state
    bool mouseDown_;
    int  lastMouseX_, lastMouseY_;

    // ── Private helpers ───────────────────────────────────────
    void setupOpenGL();
    void setupLighting();
    void updateLights();
    void checkConditions();
    void resetGame();
    float getTime() const;   // seconds since start (via glutGet)

    // ── HUD rendering ─────────────────────────────────────────
    void drawHUD();
    void drawOverlay(const char* title, const char* subtitle);
    void drawText2D (float x, float y, const char* text,
                     float r = 1.f, float g = 1.f, float b = 1.f);
    void begin2D();
    void end2D();
};
