#pragma once
/*
 * Primitives.h
 * ============================================================
 * Drawing functions for fundamental 3D shapes (Lecture 004/005).
 * All shapes are centred at the local origin (0,0,0).
 * Call from inside a glPushMatrix/glPopMatrix block to position them.
 *
 * Shapes are defined by specifying vertex positions and surface normals.
 * Normals are essential for the lighting equation: N·L determines the
 * diffuse shading.  Smooth normals (per-vertex) give Gouraud shading;
 * flat normals (per-face) give flat shading.
 * ============================================================
 */

namespace Primitives {

// ── Axis-aligned box centred at origin ────────────────────────
// Flat normals per face (6 quads)
void drawCube(float width, float height, float depth);

// ── UV sphere (latitude/longitude mesh) ───────────────────────
// Smooth normals computed from the sphere equation: N = pos / radius
void drawSphere(float radius, int slices = 18, int stacks = 18);

// ── Cylinder (body + two caps) along Y axis ───────────────────
// Centred vertically: bottom at y=-height/2, top at y=+height/2
void drawCylinder(float radius, float height, int slices = 16);

// ── Cone (base at bottom, apex at top) along Y axis ───────────
// Centred vertically
void drawCone(float baseRadius, float height, int slices = 16);

// ── Square pyramid (flat base + 4 triangular faces) ───────────
// Centred vertically; base is a square of side 'base'
void drawPyramid(float base, float height);

// ── Flat floor quad on XZ plane (y=0) ─────────────────────────
void drawFloor(float width, float depth, int divisions = 1);

// ── Torus (ring shape) ────────────────────────────────────────
// innerR = tube radius, outerR = ring radius
void drawTorus(float innerR, float outerR, int sides = 12, int rings = 20);

// ── Debug: draw X (red) Y (green) Z (blue) axes ───────────────
void drawAxes(float length = 1.f);

} // namespace Primitives
