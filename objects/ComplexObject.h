#pragma once
/*
 * ComplexObject.h
 * ============================================================
 * Composite 3D objects built from primitive shapes.
 * Demonstrates HIERARCHICAL MODELLING (Lecture 006):
 *   Each part is drawn with its own local transform inside the
 *   parent object's push/pop matrix block.
 *
 * Objects in this file:
 *   drawGem        – spinning octahedron (2 pyramids + torus ring)
 *   drawPlayerBall – sphere body + eye child (hierarchy demo)
 *   drawGuard      – humanoid: body + head + 4 limbs with walk anim
 *   drawWallBlock  – coloured cube wall segment
 *   drawGoalStar   – 6-spike 3D star (6 cones + central sphere)
 * ============================================================
 */

namespace ComplexObjects {

// Collectible gem: two pyramids joined base-to-base with a gold ring
// spinAngle drives the rotation animation (degrees, updated per frame)
void drawGem(float size, float spinAngle);

// Player avatar: green sphere body + white eye (hierarchical child)
// rollAngle makes the ball appear to roll as it moves
// faceDirY is the yaw so the eye always faces forward
void drawPlayerBall(float radius, float rollAngle, float faceDirY);

// NPC guard: humanoid shape with walk-cycle animation
// animTime drives the limb swing (seconds elapsed)
void drawGuard(float animTime);

// Maze wall cube segment
void drawWallBlock(float width, float height, float depth);

// Goal object: 6-spike star + spinning torus ring
void drawGoalStar(float size, float spinAngle);

} // namespace ComplexObjects
