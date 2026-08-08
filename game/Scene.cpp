/*
 * Scene.cpp
 * ============================================================
 * World rendering and game-entity management.
 *
 * Coordinate system:
 *   Cell (row, col) → world position:
 *     x = (col - MAZE_W/2) * cellSize      (left/right)
 *     z = (row - MAZE_H/2) * cellSize      (forward/back)
 *     y = 0 (floor level)
 *
 *   Inverse (world → cell):
 *     col = round(x / cellSize + MAZE_W/2)
 *     row = round(z / cellSize + MAZE_H/2)
 * ============================================================
 */

#include "Scene.h"
#include "../objects/ComplexObject.h"
#include "../objects/Primitives.h"
#include "../graphics/Light.h"
#include "../graphics/Transform.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ── Maze layout ───────────────────────────────────────────────────────────────
// 1 = solid wall block, 0 = open corridor
static const int MAZE_TEMPLATE[Scene::MAZE_H][Scene::MAZE_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,1,0,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,0,1,1,0,1,0,0,0,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,0,1,0,1,0,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,1,1,1,1,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ── Constructor ───────────────────────────────────────────────────────────────
Scene::Scene()
    : goalPos_(0, 0.5f, 0)
    , goalSpinAngle_(0)
    , collectedGems_(0)
    , cellSize_(2.f)
    , wallHeight_(2.2f)
{}

// ── Init / reset ──────────────────────────────────────────────────────────────
void Scene::init() {
    buildMaze();
    gems_.clear();
    guards_.clear();
    collectedGems_ = 0;
    goalSpinAngle_ = 0;
    placeGems();
    placeGuards();
    goalPos_   = cellCenter(13, 13);
    goalPos_.y = 0.6f;
}

void Scene::buildMaze() {
    for (int r = 0; r < MAZE_H; r++)
        for (int c = 0; c < MAZE_W; c++)
            maze_[r][c] = MAZE_TEMPLATE[r][c];
}

// ── Coordinate helpers ────────────────────────────────────────────────────────
Vec3 Scene::cellCenter(int row, int col) const {
    return Vec3(
        (float)(col - MAZE_W/2) * cellSize_,
        0.f,
        (float)(row - MAZE_H/2) * cellSize_
    );
}

bool Scene::isWallCell(int row, int col) const {
    if (row < 0 || row >= MAZE_H || col < 0 || col >= MAZE_W) return true;
    return maze_[row][col] == 1;
}

Vec3 Scene::getPlayerStart() const {
    return cellCenter(1, 1) + Vec3(0, 0.5f, 0);
}

// ── Wall collision ────────────────────────────────────────────────────────────
bool Scene::isWall(const Vec3& pos, float radius) const {
    // Sample 5 points on the bounding circle footprint (+center)
    const float offsets[5][2] = {
        {0,       0      },
        { radius, 0      },
        {-radius, 0      },
        {0,       radius },
        {0,      -radius }
    };
    for (auto& off : offsets) {
        float wx = pos.x + off[0];
        float wz = pos.z + off[1];
        // Inverse coord: col = round(wx / cellSize + MAZE_W/2)
        int col = (int)std::round(wx / cellSize_ + (float)(MAZE_W / 2));
        int row = (int)std::round(wz / cellSize_ + (float)(MAZE_H / 2));
        if (isWallCell(row, col)) return true;
    }
    return false;
}

// ── Gem placement ─────────────────────────────────────────────────────────────
void Scene::placeGems() {
    // Hand-picked open corridor cells for gems
    struct RC { int r, c; };
    const RC spots[] = {
        {1,3},{1,7},{1,11},
        {3,3},{3,7},{3,11},
        {5,5},{5,7},
        {7,1},{7,7},{7,13},
        {9,3},{9,7},{9,11},
        {11,3},{11,7},{11,11},
        {13,3},{13,7}
    };
    for (auto& s : spots) {
        if (maze_[s.r][s.c] == 0) {
            Gem g;
            g.position  = cellCenter(s.r, s.c) + Vec3(0, 0.65f, 0);
            g.collected = false;
            g.spinAngle = (float)(std::rand() % 360);
            gems_.push_back(g);
        }
    }
}

// ── Guard placement ───────────────────────────────────────────────────────────
void Scene::placeGuards() {
    struct GPat { int r1,c1, r2,c2; float speed; };
    const GPat patrols[] = {
        { 1, 2,  1, 5,  2.2f},
        { 7, 1,  7, 5,  2.8f},
        { 7, 9,  7,13,  2.5f},
        {13, 3, 13, 9,  2.0f},
        { 3,13,  9,13,  3.0f},
    };
    for (auto& p : patrols) {
        Guard g;
        g.waypointA   = cellCenter(p.r1, p.c1);  g.waypointA.y = 0.5f;
        g.waypointB   = cellCenter(p.r2, p.c2);  g.waypointB.y = 0.5f;
        g.position    = g.waypointA;
        g.yaw         = 0;
        g.speed       = p.speed;
        g.animTime    = (float)(std::rand() % 100) * 0.01f; // offset so not all in sync
        g.goingToB    = true;
        g.boundRadius = 0.45f;
        guards_.push_back(g);
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void Scene::update(float dt) {
    // Spin gems (path animation / constant rotation)
    for (auto& gem : gems_)
        if (!gem.collected) gem.spinAngle += 100.f * dt;

    // Goal pulses/spins
    goalSpinAngle_ += 130.f * dt;

    // Guards: patrol between waypoints (path animation)
    for (auto& guard : guards_) {
        Vec3& target = guard.goingToB ? guard.waypointB : guard.waypointA;
        Vec3 dir = target - guard.position;
        float dist = dir.length();

        if (dist < 0.12f) {
            guard.goingToB = !guard.goingToB;
        } else {
            Vec3 step = dir.normalize() * (guard.speed * dt);
            // Axis-separated wall collision: try X then Z independently so the
            // guard slides along walls and can never pass through them.
            Vec3 moved(0, 0, 0);
            Vec3 testX = guard.position + Vec3(step.x, 0, 0);
            if (!isWall(testX, guard.boundRadius * 0.85f)) {
                guard.position.x = testX.x;
                moved.x = step.x;
            }
            Vec3 testZ = guard.position + Vec3(0, 0, step.z);
            if (!isWall(testZ, guard.boundRadius * 0.85f)) {
                guard.position.z = testZ.z;
                moved.z = step.z;
            }
            // Face direction of actual travel (eyes are at local +Z)
            if (moved.x*moved.x + moved.z*moved.z > 1e-6f) {
                guard.yaw = radToDeg(std::atan2(moved.x, moved.z));
            }
        }
        guard.animTime += dt;
    }
}

// ── Camera collision resolution ───────────────────────────────────────────────
Vec3 Scene::resolveCameraEye(const Vec3& from, const Vec3& to) const {
    Vec3 flat = to - from;
    flat.y = 0;
    float flatDist = flat.length();
    if (flatDist < 1e-4f) return to;

    const float camRadius = 0.35f;
    const float minH      = 1.1f;   // never crowd the player
    const int   steps     = 48;

    // Walk horizontally from the DESIRED eye back toward the player and take
    // the farthest position that is free of walls. Starting from the far side
    // guarantees we never return the player position itself (which previously
    // pinned the camera inside the player ball → green screen + stuck view).
    float bestT = 0.f;
    for (int i = steps; i >= 0; i--) {
        float t = (float)i / (float)steps;
        Vec3 pt = from + Vec3(flat.x * t, 0, flat.z * t);
        pt.y = to.y;                 // keep the elevated camera height
        if (!isWall(pt, camRadius)) { bestT = t; break; }
    }

    Vec3 eye = from + Vec3(flat.x * bestT, 0, flat.z * bestT);
    eye.y = to.y;

    // If a wall forces the camera very close to the player, keep a minimum
    // horizontal separation and let the elevated height look over the wall.
    Vec3 hFlat = eye - from;
    hFlat.y = 0;
    float h = hFlat.length();
    if (h < minH) {
        Vec3 n = (h > 1e-4f) ? hFlat * (1.f / h) : Vec3(0, 0, 1);
        eye = from + n * minH;
        eye.y = to.y;
    }
    return eye;
}

// ── Collision checks ──────────────────────────────────────────────────────────
bool Scene::checkGemCollision(const Vec3& playerPos, float playerRadius) {
    bool any = false;
    for (auto& gem : gems_) {
        if (gem.collected) continue;
        Vec3 diff = gem.position - playerPos;
        float dist2  = diff.dot(diff);
        float radSum = 0.4f + playerRadius;
        if (dist2 < radSum * radSum) {
            gem.collected = true;
            collectedGems_++;
            any = true;
        }
    }
    return any;
}

bool Scene::checkGuardCollision(const Vec3& playerPos, float playerRadius) const {
    for (const auto& guard : guards_) {
        Vec3 diff = guard.position - playerPos;
        diff.y = 0;                           // ignore height difference
        float dist2  = diff.dot(diff);
        float radSum = guard.boundRadius + playerRadius;
        if (dist2 < radSum * radSum) return true;
    }
    return false;
}

bool Scene::checkGoalCollision(const Vec3& playerPos, float playerRadius) const {
    Vec3 diff = goalPos_ - playerPos;
    diff.y = 0;
    float dist2  = diff.dot(diff);
    float radSum = 0.75f + playerRadius;
    return dist2 < radSum * radSum;
}

// ── Draw ──────────────────────────────────────────────────────────────────────
void Scene::draw() {
    drawSkybox();
    drawFloorPlane();
    drawMazeWalls();
    drawGems();
    drawGuards();
    drawGoal();
}

void Scene::drawSkybox() {
    /*
     * Simple gradient sky cube.
     * Rendered without lighting so colour is unaffected by lights.
     * Large enough (80 units) that it always surrounds the camera.
     */
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);             // sky must not be fogged
    glDepthMask(GL_FALSE);         // don't write to depth buffer

    const float S = 80.f;

    glBegin(GL_QUADS);
    // Top (deep blue)
    glColor3f(0.10f, 0.25f, 0.65f);
    glVertex3f(-S, S,-S); glVertex3f( S, S,-S);
    glVertex3f( S, S, S); glVertex3f(-S, S, S);

    // Front (horizon gradient: lighter at bottom)
    glColor3f(0.15f, 0.35f, 0.75f); glVertex3f(-S, S, S); glVertex3f( S, S, S);
    glColor3f(0.45f, 0.65f, 0.95f); glVertex3f( S,-S, S); glVertex3f(-S,-S, S);

    // Back
    glColor3f(0.15f, 0.35f, 0.75f); glVertex3f( S, S,-S); glVertex3f(-S, S,-S);
    glColor3f(0.45f, 0.65f, 0.95f); glVertex3f(-S,-S,-S); glVertex3f( S,-S,-S);

    // Left
    glColor3f(0.15f, 0.35f, 0.75f); glVertex3f(-S, S,-S); glVertex3f(-S, S, S);
    glColor3f(0.45f, 0.65f, 0.95f); glVertex3f(-S,-S, S); glVertex3f(-S,-S,-S);

    // Right
    glColor3f(0.15f, 0.35f, 0.75f); glVertex3f( S, S, S); glVertex3f( S, S,-S);
    glColor3f(0.45f, 0.65f, 0.95f); glVertex3f( S,-S,-S); glVertex3f( S,-S, S);
    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

void Scene::drawFloorPlane() {
    materialFloor();
    float totalW = MAZE_W * cellSize_;
    float totalD = MAZE_H * cellSize_;
    Primitives::drawFloor(totalW, totalD, 30);

    // Navigation grid: faint lines along every cell boundary help the player
    // read the maze and see where they are.
    glDisable(GL_LIGHTING);
    glColor3f(0.20f, 0.42f, 0.20f);
    glLineWidth(1.f);
    glBegin(GL_LINES);
    float halfW = totalW * 0.5f;
    float halfD = totalD * 0.5f;
    for (int i = 0; i <= MAZE_W; i++) {
        float x = -halfW + i * cellSize_;
        glVertex3f(x, 0.01f, -halfD);
        glVertex3f(x, 0.01f,  halfD);
    }
    for (int j = 0; j <= MAZE_H; j++) {
        float z = -halfD + j * cellSize_;
        glVertex3f(-halfW, 0.01f, z);
        glVertex3f( halfW, 0.01f, z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void Scene::drawMazeWalls() {
    for (int r = 0; r < MAZE_H; r++) {
        for (int c = 0; c < MAZE_W; c++) {
            if (maze_[r][c] == 1) {
                Vec3 pos = cellCenter(r, c);
                glPushMatrix();
                glTranslatef(pos.x, wallHeight_ * 0.5f, pos.z);
                ComplexObjects::drawWallBlock(cellSize_, wallHeight_, cellSize_);
                glPopMatrix();
            }
        }
    }
}

void Scene::drawGems() {
    for (const auto& gem : gems_) {
        if (gem.collected) continue;
        glPushMatrix();
        glTranslatef(gem.position.x, gem.position.y, gem.position.z);
        ComplexObjects::drawGem(0.32f, gem.spinAngle);
        glPopMatrix();
    }
}

void Scene::drawGuards() {
    for (const auto& guard : guards_) {
        glPushMatrix();
        glTranslatef(guard.position.x, guard.position.y, guard.position.z);
        glRotatef(guard.yaw, 0, 1, 0);
        ComplexObjects::drawGuard(guard.animTime);
        glPopMatrix();
    }
}

void Scene::drawGoal() {
    // Platform
    materialGold();
    glPushMatrix();
        glTranslatef(goalPos_.x, 0.06f, goalPos_.z);
        Primitives::drawCylinder(0.65f, 0.12f, 20);
    glPopMatrix();
    // Star
    glPushMatrix();
        glTranslatef(goalPos_.x, goalPos_.y + 0.3f, goalPos_.z);
        ComplexObjects::drawGoalStar(0.42f, goalSpinAngle_);
    glPopMatrix();
}
