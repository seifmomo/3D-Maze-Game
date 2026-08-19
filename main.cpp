/*
 * main.cpp
 * ============================================================
 * Entry point for 3D Maze Game (Computer Graphics Project).
 * Initialises GLUT window and registers global callback wrappers.
 * ============================================================
 */

#include "game/Game.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/freeglut.h>
#include <iostream>

// Global Game instance pointer for GLUT C-style static callbacks
static Game* g_gameInstance = nullptr;

// ── GLUT Callback Wrappers ────────────────────────────────────────────────────
static void displayCallback() {
    if (g_gameInstance) g_gameInstance->display();
}

static void reshapeCallback(int w, int h) {
    if (g_gameInstance) g_gameInstance->reshape(w, h);
}

static void idleCallback() {
    if (g_gameInstance) g_gameInstance->idle();
}

static void keyDownCallback(unsigned char key, int x, int y) {
    if (g_gameInstance) g_gameInstance->keyDown(key, x, y);
}

static void keyUpCallback(unsigned char key, int x, int y) {
    if (g_gameInstance) g_gameInstance->keyUp(key, x, y);
}

static void specialDownCallback(int key, int x, int y) {
    if (g_gameInstance) g_gameInstance->specialDown(key, x, y);
}

static void specialUpCallback(int key, int x, int y) {
    if (g_gameInstance) g_gameInstance->specialUp(key, x, y);
}

static void mouseButtonCallback(int button, int state, int x, int y) {
    if (g_gameInstance) g_gameInstance->mouseButton(button, state, x, y);
}

static void mouseMotionCallback(int x, int y) {
    if (g_gameInstance) g_gameInstance->mouseMotion(x, y);
}

// ── Main Entry Point ─────────────────────────────────────────────────────────
int main(int argc, char** argv) {
    std::cout << "====================================================\n";
    std::cout << "          3D MAZE GAME - COMPUTER GRAPHICS          \n";
    std::cout << "====================================================\n";
    std::cout << " Controls:\n";
    std::cout << "  - WASD / Arrow Keys : Move & Turn Player Ball\n";
    std::cout << "  - Mouse Drag        : Orbit / Look Camera\n";
    std::cout << "  - V or 1/2/3        : Switch Camera (1st/3rd/Top-Down)\n";
    std::cout << "  - L                 : Toggle Sun Light\n";
    std::cout << "  - G                 : Toggle Debug Axes\n";
    std::cout << "  - Space             : Pause / Resume Game\n";
    std::cout << "  - R                 : Reset / Restart Game\n";
    std::cout << "  - ESC               : Exit\n";
    std::cout << "====================================================\n\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Maze Game - Computer Graphics Project");

#ifdef _WIN32
    // Force keyboard focus to the game window at startup, otherwise the
    // console window keeps focus and WASD/arrow keys do nothing.
    HWND hwnd = FindWindowA(NULL, "3D Maze Game - Computer Graphics Project");
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
    }
#endif

    Game game;
    g_gameInstance = &game;
    game.init(1024, 768);

    // Register Callbacks
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutIdleFunc(idleCallback);

    glutKeyboardFunc(keyDownCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);

    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseMotionCallback);

    // Start GLUT main loop
    glutMainLoop();

    return 0;
}
