/*
 * ComplexObject.cpp
 * ============================================================
 * All complex objects are built by:
 *   1. glPushMatrix()
 *   2. Apply a local transform (translate/rotate/scale)
 *   3. Set a material (Phong coefficients)
 *   4. Draw a primitive
 *   5. glPopMatrix()
 * Nesting these blocks creates a parent-child hierarchy.
 * ============================================================
 */

#include "ComplexObject.h"
#include "Primitives.h"
#include "../graphics/Light.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ComplexObjects {

// ── Gem ───────────────────────────────────────────────────────────────────────
/*
 * Hierarchy:
 *   gem root (spin + tilt)
 *     ├─ top pyramid
 *     ├─ bottom pyramid (rotated 180°)
 *     └─ equatorial torus ring
 */
void drawGem(float size, float spinAngle) {
    glPushMatrix();
    glRotatef(spinAngle, 0, 1, 0);      // ANIMATION: spin around Y each frame
    glRotatef(20.f, 1, 0, 1);           // fixed tilt for visual interest

    // Top pyramid (apex up)
    materialRuby();
    glPushMatrix();
        glTranslatef(0, size * 0.25f, 0);
        Primitives::drawPyramid(size * 0.65f, size * 0.8f);
    glPopMatrix();

    // Bottom pyramid (apex down → rotate 180° around Z)
    glPushMatrix();
        glTranslatef(0, -size * 0.25f, 0);
        glRotatef(180.f, 0, 0, 1);
        Primitives::drawPyramid(size * 0.65f, size * 0.65f);
    glPopMatrix();

    // Gold equatorial ring (torus on XZ plane)
    materialGold();
    glPushMatrix();
        glRotatef(90.f, 1, 0, 0);       // torus is in XY by default → rotate to XZ
        Primitives::drawTorus(0.04f, size * 0.33f, 8, 16);
    glPopMatrix();

    glPopMatrix();
}

// ── Player ball ───────────────────────────────────────────────────────────────
/*
 * Hierarchy:
 *   body sphere (roll animation, face direction)
 *     └─ eye sphere  ← child rendered in body-local space
 *          └─ pupil  ← grandchild
 * This demonstrates a 3-level object hierarchy.
 */
void drawPlayerBall(float radius, float rollAngle, float faceDirY) {
    glPushMatrix();

    // Rotate body so the eye faces the player's movement direction
    glRotatef(faceDirY, 0, 1, 0);

    // Roll animation: rotate about the lateral axis so the ball appears
    // to roll forward in the direction it travels (visual feedback)
    glRotatef(rollAngle, 1, 0, 0);

    // ── Body (parent node) ────────────────────────────────────
    materialPlayer();
    Primitives::drawSphere(radius, 20, 20);

    // ── Eye (child node – inherits body rotation) ─────────────
    glPushMatrix();
        glTranslatef(0, radius * 0.3f, radius * 0.78f);

        // White sclera
        setMaterial({0.9f,0.9f,0.9f,1},{0.9f,0.9f,0.9f,1},
                    {0.5f,0.5f,0.5f,1}, 30.f);
        Primitives::drawSphere(radius * 0.28f, 10, 10);

        // ── Pupil (grandchild node) ───────────────────────────
        glPushMatrix();
            glTranslatef(0, 0, radius * 0.25f);
            setMaterial({0,0,0,1},{0.05f,0.05f,0.05f,1},{0,0,0,1}, 0);
            Primitives::drawSphere(radius * 0.14f, 8, 8);
        glPopMatrix();

    glPopMatrix();

    glPopMatrix();
}

// ── NPC Guard ─────────────────────────────────────────────────────────────────
/*
 * Hierarchy:
 *   body cylinder (parent)
 *     ├─ head sphere
 *     │    ├─ left eye
 *     │    └─ right eye
 *     ├─ left arm  (swings forward/back with sin(animTime))
 *     ├─ right arm (opposite phase)
 *     ├─ left leg
 *     └─ right leg
 *
 * ANIMATION: limbs swing with sin(animTime * speed) → walk cycle
 */
void drawGuard(float animTime) {
    float swing = std::sin(animTime * 5.f) * 25.f;   // degrees

    materialGuard();

    // ── Body (parent) ─────────────────────────────────────────
    glPushMatrix();
    Primitives::drawCylinder(0.22f, 0.75f, 12);

    // ── Head (child of body) ──────────────────────────────────
    glPushMatrix();
        glTranslatef(0, 0.6f, 0);
        materialGuard();
        Primitives::drawSphere(0.23f, 12, 12);

        // Eyes (children of head)
        setMaterial({1,1,0,1},{1,1,0,1},{1,1,1,1}, 16);
        glPushMatrix();
            glTranslatef( 0.11f, 0.06f, 0.20f);
            Primitives::drawSphere(0.055f, 6, 6);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(-0.11f, 0.06f, 0.20f);
            Primitives::drawSphere(0.055f, 6, 6);
        glPopMatrix();
    glPopMatrix();

    // ── Left arm ──────────────────────────────────────────────
    materialGuard();
    glPushMatrix();
        glTranslatef(-0.32f, 0.12f, 0);
        glRotatef(swing, 1, 0, 0);           // walk animation
        Primitives::drawCylinder(0.09f, 0.45f, 8);
    glPopMatrix();

    // ── Right arm (opposite phase) ────────────────────────────
    glPushMatrix();
        glTranslatef( 0.32f, 0.12f, 0);
        glRotatef(-swing, 1, 0, 0);
        Primitives::drawCylinder(0.09f, 0.45f, 8);
    glPopMatrix();

    // ── Left leg ──────────────────────────────────────────────
    glPushMatrix();
        glTranslatef(-0.13f, -0.62f, 0);
        glRotatef(-swing, 1, 0, 0);
        Primitives::drawCylinder(0.09f, 0.50f, 8);
    glPopMatrix();

    // ── Right leg (opposite phase) ────────────────────────────
    glPushMatrix();
        glTranslatef( 0.13f, -0.62f, 0);
        glRotatef( swing, 1, 0, 0);
        Primitives::drawCylinder(0.09f, 0.50f, 8);
    glPopMatrix();

    glPopMatrix();  // end body
}

// ── Wall block ────────────────────────────────────────────────────────────────
void drawWallBlock(float width, float height, float depth) {
    materialWall();
    Primitives::drawCube(width, height, depth);

    // Subtle edge cap: tiny darker cubes at top edges for visual depth
    setMaterial({0.05f,0.05f,0.1f,1},{0.15f,0.15f,0.25f,1},
                {0,0,0,1}, 5);
    glPushMatrix();
        glTranslatef(0, height * 0.5f - 0.04f, 0);
        Primitives::drawCube(width, 0.08f, depth);
    glPopMatrix();
}

// ── Goal star ─────────────────────────────────────────────────────────────────
/*
 * 6 conical spikes pointing along ±X, ±Y, ±Z axes.
 * Central sphere at origin.
 * Entire shape spins around Y (and wobbles around X) for animation.
 */
void drawGoalStar(float size, float spinAngle) {
    glPushMatrix();
    glRotatef(spinAngle,        0, 1, 0);
    glRotatef(spinAngle * 0.4f, 1, 0, 0);

    materialGold();

    // ─ +Y spike ─
    glPushMatrix();
        glTranslatef(0, size * 0.25f, 0);
        Primitives::drawCone(size * 0.22f, size * 0.8f, 10);
    glPopMatrix();
    // ─ -Y spike ─
    glPushMatrix();
        glTranslatef(0, -size * 0.25f, 0);
        glRotatef(180, 1, 0, 0);
        Primitives::drawCone(size * 0.22f, size * 0.8f, 10);
    glPopMatrix();
    // ─ +X spike ─
    glPushMatrix();
        glTranslatef(size * 0.25f, 0, 0);
        glRotatef(90, 0, 0, -1);
        Primitives::drawCone(size * 0.22f, size * 0.8f, 10);
    glPopMatrix();
    // ─ -X spike ─
    glPushMatrix();
        glTranslatef(-size * 0.25f, 0, 0);
        glRotatef(90, 0, 0, 1);
        Primitives::drawCone(size * 0.22f, size * 0.8f, 10);
    glPopMatrix();
    // ─ +Z spike ─
    glPushMatrix();
        glTranslatef(0, 0, size * 0.25f);
        glRotatef(90, 1, 0, 0);
        Primitives::drawCone(size * 0.22f, size * 0.8f, 10);
    glPopMatrix();
    // ─ -Z spike ─
    glPushMatrix();
        glTranslatef(0, 0, -size * 0.25f);
        glRotatef(-90, 1, 0, 0);
        Primitives::drawCone(size * 0.22f, size * 0.8f, 10);
    glPopMatrix();

    // Central sphere
    Primitives::drawSphere(size * 0.22f, 12, 12);

    // Spinning equatorial ring
    glPushMatrix();
        glRotatef(-spinAngle * 2.f, 0, 1, 0);   // counter-spin for contrast
        glRotatef(90, 1, 0, 0);
        Primitives::drawTorus(0.05f, size * 0.4f, 8, 20);
    glPopMatrix();

    glPopMatrix();
}

} // namespace ComplexObjects
