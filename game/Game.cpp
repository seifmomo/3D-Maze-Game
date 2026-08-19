/*
 * Game.cpp
 * ============================================================
 * Top-level game logic, rendering, HUD, input handling, and OpenGL setup.
 * Implements camera mode toggles (1: First Person, 2: Third Person, 3: Top-Down, V: Cycle),
 * sun light toggle (L), debug world axes (G), game timer, gem score tracking,
 * win/lose screens, and HUD overlays.
 * ============================================================
 */

#include "Game.h"
#include "../graphics/Light.h"
#include "../graphics/Transform.h"
#include "../objects/Primitives.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#ifndef GL_LIGHTMODEL_AMBIENT
#  define GL_LIGHTMODEL_AMBIENT 0x0B53
#endif
#ifndef GL_MULTISAMPLE
#  define GL_MULTISAMPLE 0x809D
#endif
#include <GL/freeglut.h>
#include <cstdio>
#include <cstring>
#include <cmath>

// ── Constructor ───────────────────────────────────────────────────────────────
Game::Game()
    : sunLight_  (GL_LIGHT0, LightType::DIRECTIONAL)
    , pointLight_(GL_LIGHT1, LightType::POINT)
    , state_(GameState::PLAYING)
    , timer_(90.0f)
    , totalTime_(90.0f)
    , score_(0)
    , lives_(3)
    , windowW_(800)
    , windowH_(600)
    , lastFrameTime_(0.0f)
    , deltaTime_(0.0f)
    , mouseDown_(false)
    , lastMouseX_(0)
    , lastMouseY_(0)
    , showAxes_(false)
{}

// ── Init ──────────────────────────────────────────────────────────────────────
void Game::init(int w, int h) {
    windowW_ = w;
    windowH_ = h;
    
    setupOpenGL();
    setupLighting();
    
    scene_.init();
    player_.setScene(&scene_);
    resetGame();
    
    lastFrameTime_ = getTime();
}

void Game::resetGame() {
    state_     = GameState::PLAYING;
    timer_     = totalTime_;
    score_     = 0;
    lives_     = 3;
    
    scene_.init();
    player_.init(scene_.getPlayerStart());
    camera_.setMode(CameraMode::THIRD_PERSON);
}

float Game::getTime() const {
    return (float)glutGet(GLUT_ELAPSED_TIME) * 0.001f;
}

// ── OpenGL Setup ──────────────────────────────────────────────────────────────
void Game::setupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE); // Ensure scaled matrices maintain unit length normals

    // Smooth shading (Gouraud / Blinn-Phong)
    glShadeModel(GL_SMOOTH);

    // Full-screen multisampling for smoother edges
    glEnable(GL_MULTISAMPLE);

    // Distance fog for depth perception
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    float fogColor[4] = { 0.12f, 0.18f, 0.30f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 18.f);
    glFogf(GL_FOG_END,   55.f);

    // Global ambient illumination
    float globalAmbient[] = { 0.2f, 0.2f, 0.25f, 1.0f };
    glLightModelfv(GL_LIGHTMODEL_AMBIENT, globalAmbient);

    // Background clear color
    glClearColor(0.1f, 0.15f, 0.25f, 1.0f);
}

void Game::setupLighting() {
    // Light 0: Directional Sun Light
    sunLight_.setDirection({ 0.5f, 1.0f, 0.7f });
    sunLight_.setAmbient  ({ 0.15f, 0.15f, 0.2f, 1.0f });
    sunLight_.setDiffuse  ({ 0.85f, 0.85f, 0.75f, 1.0f });
    sunLight_.setSpecular ({ 0.9f, 0.9f, 0.8f, 1.0f });
    sunLight_.enable();

    // Light 1: Point Light (Torch near player)
    pointLight_.setPosition(scene_.getPlayerStart() + Vec3(0, 1.5f, 0));
    pointLight_.setAmbient  ({ 0.1f, 0.05f, 0.0f, 1.0f });
    pointLight_.setDiffuse  ({ 1.0f, 0.7f, 0.3f, 1.0f });
    pointLight_.setSpecular ({ 1.0f, 0.8f, 0.5f, 1.0f });
    pointLight_.setAttenuation(1.0f, 0.07f, 0.014f);
    pointLight_.enable();
}

void Game::updateLights() {
    // Keep torch / point light hovering near player position
    Vec3 pPos = player_.getPosition();
    pointLight_.setPosition({ pPos.x, pPos.y + 1.2f, pPos.z });
}

// ── Reshape ───────────────────────────────────────────────────────────────────
void Game::reshape(int w, int h) {
    windowW_ = w > 0 ? w : 1;
    windowH_ = h > 0 ? h : 1;
    glViewport(0, 0, windowW_, windowH_);
    camera_.setPerspective(60.0f, (float)windowW_ / (float)windowH_, 0.1f, 200.0f);
}

// ── Main Game Loop Functions ──────────────────────────────────────────────────
void Game::idle() {
    float currentTime = getTime();
    deltaTime_ = currentTime - lastFrameTime_;
    lastFrameTime_ = currentTime;
    
    // Clamp delta time to prevent large steps when dragging window
    if (deltaTime_ > 0.1f) deltaTime_ = 0.1f;

    if (state_ == GameState::PLAYING) {
        timer_ -= deltaTime_;
        if (timer_ <= 0.0f) {
            timer_ = 0.0f;
            state_ = GameState::LOSE;
        }

        player_.update(deltaTime_,
                       camera_.getForward(),
                       camera_.getRight());
        scene_.update(deltaTime_);
        camera_.update(player_.getPosition(), player_.getYaw(), deltaTime_,
                       player_.isMoving());

        // Keep the orbit camera out of maze walls (no clipping, no green screen)
        if (camera_.getMode() == CameraMode::THIRD_PERSON) {
            camera_.snapEye(scene_.resolveCameraEye(player_.getPosition(),
                                                    camera_.getEye()));
        }

        updateLights();
        
        updateGameRules();
    }

    glutPostRedisplay();
}

void Game::updateGameRules() {
    Vec3 pPos = player_.getPosition();
    float pRad = player_.getRadius();

    // Gem pickup
    if (scene_.checkGemCollision(pPos, pRad)) {
        score_ += 100;
    }

    // Guard hit check
    if (scene_.checkGuardCollision(pPos, pRad)) {
        lives_--;
        if (lives_ <= 0) {
            state_ = GameState::LOSE;
        } else {
            // Respawn player at start position
            player_.respawn(scene_.getPlayerStart());
        }
    }

    // Goal reached check
    if (scene_.checkGoalCollision(pPos, pRad)) {
        score_ += (int)(timer_ * 10); // Bonus score for remaining time
        state_ = GameState::WIN;
    }
}

// ── Display ───────────────────────────────────────────────────────────────────
void Game::display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Apply Camera Projection & View
    camera_.applyProjection(windowW_, windowH_);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera_.applyView();

    // Apply lights in View space
    sunLight_.apply();
    pointLight_.apply();

    // Debug world axes (X=red, Y=green, Z=blue) — toggle with G
    if (showAxes_) {
        Primitives::drawAxes(8.f);
    }

    // Render Scene & Player
    scene_.draw();

    // In 3rd person or top-down view, render player object. In 1st person, don't render self.
    if (camera_.getMode() != CameraMode::FIRST_PERSON) {
        player_.draw();
    }

    // Render 2D HUD / Overlays
    drawHUD();

    glutSwapBuffers();
}

// ── HUD and Text Rendering ───────────────────────────────────────────────────
void Game::begin2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowW_, 0, windowH_);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

void Game::end2D() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void Game::drawText2D(float x, float y, const char* text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void Game::drawHUD() {
    begin2D();

    char buf[128];
    // Timer & Score
    snprintf(buf, sizeof(buf), "TIME: %.1fs   SCORE: %d   LIVES: %d", timer_, score_, lives_);
    drawText2D(20, windowH_ - 30, buf, 1.0f, 1.0f, 1.0f);

    // Gems Count
    snprintf(buf, sizeof(buf), "GEMS: %d / %d", scene_.getCollectedGems(), scene_.getTotalGems());
    drawText2D(20, windowH_ - 55, buf, 0.9f, 0.8f, 0.2f);

    // Camera Mode Display
    const char* modeStr = "3RD PERSON";
    if (camera_.getMode() == CameraMode::FIRST_PERSON) modeStr = "1ST PERSON";
    else if (camera_.getMode() == CameraMode::TOP_DOWN) modeStr = "TOP-DOWN (ORTHO)";

    snprintf(buf, sizeof(buf), "CAMERA [V / 1-3]: %s", modeStr);
    drawText2D(windowW_ - 280, windowH_ - 30, buf, 0.4f, 0.9f, 1.0f);

    // Controls tip
    drawText2D(20, 20, "WASD: Move | Mouse Drag: Look/Orbit | Space: Pause | R: Reset | G: Axes", 0.7f, 0.7f, 0.7f);

    // Render Overlays for PAUSED / WIN / LOSE states
    if (state_ == GameState::PAUSED) {
        drawOverlay("GAME PAUSED", "Press SPACE to Resume");
    } else if (state_ == GameState::WIN) {
        snprintf(buf, sizeof(buf), "VICTORY! Final Score: %d", score_);
        drawOverlay(buf, "Press R to Play Again");
    } else if (state_ == GameState::LOSE) {
        drawOverlay("GAME OVER", "Press R to Restart");
    }

    end2D();
}

void Game::drawOverlay(const char* title, const char* subtitle) {
    // Dark semi-transparent box
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f((float)windowW_, 0);
        glVertex2f((float)windowW_, (float)windowH_);
        glVertex2f(0, (float)windowH_);
    glEnd();
    glDisable(GL_BLEND);

    drawText2D(windowW_ * 0.5f - 80, windowH_ * 0.5f + 20, title, 1.0f, 0.85f, 0.1f);
    drawText2D(windowW_ * 0.5f - 100, windowH_ * 0.5f - 15, subtitle, 1.0f, 1.0f, 1.0f);
}

// ── Keyboard Callback ─────────────────────────────────────────────────────────
void Game::keyDown(unsigned char key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case 'w': case 'W': player_.setMoveForward(true); break;
        case 's': case 'S': player_.setMoveBack(true);    break;
        case 'a': case 'A': player_.setMoveLeft(true);    break;
        case 'd': case 'D': player_.setMoveRight(true);   break;

        case 'q': case 'Q': player_.setTurnLeft(true);    break;
        case 'e': case 'E': player_.setTurnRight(true);   break;

        case '1': camera_.setMode(CameraMode::FIRST_PERSON); break;
        case '2': camera_.setMode(CameraMode::THIRD_PERSON); break;
        case '3': camera_.setMode(CameraMode::TOP_DOWN);     break;
        case 'v': case 'V': camera_.cycleMode();             break;

        case 'l': case 'L': 
            if (sunLight_.isEnabled()) sunLight_.disable(); 
            else sunLight_.enable(); 
            break;

        case 'g': case 'G': showAxes_ = !showAxes_; break;

        case ' ':
            if (state_ == GameState::PLAYING) state_ = GameState::PAUSED;
            else if (state_ == GameState::PAUSED) state_ = GameState::PLAYING;
            break;

        case 'r': case 'R': resetGame(); break;

        case 27: // ESC
            exit(0);
            break;
    }
}

void Game::keyUp(unsigned char key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case 'w': case 'W': player_.setMoveForward(false); break;
        case 's': case 'S': player_.setMoveBack(false);    break;
        case 'a': case 'A': player_.setMoveLeft(false);    break;
        case 'd': case 'D': player_.setMoveRight(false);   break;
        case 'q': case 'Q': player_.setTurnLeft(false);     break;
        case 'e': case 'E': player_.setTurnRight(false);    break;
    }
}

void Game::specialDown(int key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case GLUT_KEY_UP:    player_.setMoveForward(true); break;
        case GLUT_KEY_DOWN:  player_.setMoveBack(true);    break;
        case GLUT_KEY_LEFT:  player_.setMoveLeft(true);    break;
        case GLUT_KEY_RIGHT: player_.setMoveRight(true);   break;
    }
}

void Game::specialUp(int key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case GLUT_KEY_UP:    player_.setMoveForward(false); break;
        case GLUT_KEY_DOWN:  player_.setMoveBack(false);    break;
        case GLUT_KEY_LEFT:  player_.setMoveLeft(false);    break;
        case GLUT_KEY_RIGHT: player_.setMoveRight(false);   break;
    }
}

// ── Mouse Callbacks ───────────────────────────────────────────────────────────
void Game::mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseDown_ = true;
            lastMouseX_ = x;
            lastMouseY_ = y;
        } else if (state == GLUT_UP) {
            mouseDown_ = false;
            camera_.endMouseOrbit();
        }
    }
    // Mouse wheel (freeglut reports scroll as buttons 3 and 4)
    if (state == GLUT_DOWN) {
        if (button == 3) camera_.zoom(+0.8f);
        if (button == 4) camera_.zoom(-0.8f);
    }
}

void Game::mouseMotion(int x, int y) {
    if (mouseDown_) {
        int dx = x - lastMouseX_;
        int dy = y - lastMouseY_;
        lastMouseX_ = x;
        lastMouseY_ = y;

        if (camera_.getMode() == CameraMode::FIRST_PERSON) {
            // Classic mouse-look: horizontal turns the player (so WASD moves
            // where you're looking), vertical looks up/down.
            player_.addYaw(dx * 0.3f);
            camera_.addPitch(-dy * 0.3f);
        } else {
            camera_.onMouseDrag(dx, dy);
        }
    }
}
