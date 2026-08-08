#pragma once
/*
 * Scene.h
 * ============================================================
 * Manages the complete game world:
 *   - 15×15 grid maze (1=wall, 0=open corridor)
 *   - Gem collectibles with spinning animation
 *   - NPC guard patrols between waypoints
 *   - Goal object at the exit
 *   - Skybox and floor rendering
 *
 * The maze uses a 2D integer array; world coordinates are derived
 * from grid indices via:
 *   world.x = (col - MAZE_W/2) * cellSize
 *   world.z = (row - MAZE_H/2) * cellSize
 * ============================================================
 */

#include "../graphics/Transform.h"
#include <vector>

// ─────────────────────────────────────────────────────────────
struct Gem {
    Vec3  position;
    bool  collected;
    float spinAngle;   // degrees, updated each frame
};

struct Guard {
    Vec3  position;
    float yaw;         // facing direction in degrees
    float speed;
    float animTime;    // seconds elapsed for walk animation
    Vec3  waypointA, waypointB;
    bool  goingToB;
    float boundRadius;
};

// ─────────────────────────────────────────────────────────────
class Scene {
public:
    static const int MAZE_W = 15;
    static const int MAZE_H = 15;

    Scene();

    void init();            // (re)build maze, place objects
    void update(float dt);
    void draw();

    // ── Collision queries ────────────────────────────────────
    /*
     * isWall: returns true if a sphere of 'radius' centred at 'pos'
     * overlaps any wall cell in the maze grid.
     */
    bool isWall           (const Vec3& pos, float radius) const;
    bool checkGemCollision(const Vec3& playerPos, float playerRadius);
    bool checkGuardCollision(const Vec3& playerPos, float playerRadius) const;
    bool checkGoalCollision (const Vec3& playerPos, float playerRadius) const;

    // ── Camera collision ──────────────────────────────────────
    /*
     * Returns the farthest point along the segment from 'from' to 'to'
     * that is not inside a wall.  Used to stop the third-person camera
     * from clipping through maze walls (small sphere radius used for the
     * camera).
     */
    Vec3 resolveCameraEye(const Vec3& from, const Vec3& to) const;

    // ── State queries ────────────────────────────────────────
    int  getTotalGems()     const { return (int)gems_.size(); }
    int  getCollectedGems() const { return collectedGems_; }
    Vec3 getPlayerStart()   const;
    Vec3 getGoalPosition()  const { return goalPos_; }

private:
    int  maze_[MAZE_H][MAZE_W];

    std::vector<Gem>   gems_;
    std::vector<Guard> guards_;

    Vec3  goalPos_;
    float goalSpinAngle_;
    int   collectedGems_;

    float cellSize_;    // 2.0 world units per cell
    float wallHeight_;  // 2.0

    // ── Build helpers ────────────────────────────────────────
    void buildMaze();
    void placeGems();
    void placeGuards();

    // ── Draw helpers ─────────────────────────────────────────
    void drawSkybox();
    void drawFloorPlane();
    void drawMazeWalls();
    void drawGems();
    void drawGuards();
    void drawGoal();

    // ── Grid ↔ world conversion ──────────────────────────────
    Vec3 cellCenter(int row, int col) const;
    bool isWallCell(int row, int col) const;
};
