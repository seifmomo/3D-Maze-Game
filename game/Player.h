#pragma once
/*
 * Player.h
 * ============================================================
 * Manages the player avatar:
 *   - Continuous world-space position (not grid-snapped)
 *   - Movement direction computed from yaw angle
 *   - WASD + arrow key input flags (set by Game's key callbacks)
 *   - Collision detection delegated to Scene::isWall()
 *   - Ball roll animation driven by distance travelled
 *
 * Movement model (Lecture 006 – frame-based animation):
 *   Each frame: position += forward * speed * deltaTime
 *   This gives smooth, frame-rate-independent motion.
 *
 * Collision response (axis-separated):
 *   Try X movement first; if blocked, don't apply X.
 *   Try Z movement independently; if blocked, don't apply Z.
 *   This lets the player "slide" along walls.
 * ============================================================
 */

#include "../graphics/Transform.h"

class Scene; // forward declaration avoids circular include

class Player {
public:
    Player();

    void init   (const Vec3& startPos);
    void respawn(const Vec3& startPos);

    // moveFwd / moveRight are the active camera's horizontal forward and right
    // vectors, so movement is camera-relative: W always goes into the screen,
    // A/D strafe. The ball automatically faces its direction of travel.
    void update(float dt, const Vec3& moveFwd, const Vec3& moveRight);
    void draw   ();

    // ── Input flags (set true on keyDown, false on keyUp) ─────
    void setMoveForward (bool on) { moveForward_  = on; }
    void setMoveBack    (bool on) { moveBack_     = on; }
    void setMoveLeft    (bool on) { moveLeft_     = on; }
    void setMoveRight   (bool on) { moveRight_    = on; }
    void setTurnLeft    (bool on) { turnLeft_     = on; }
    void setTurnRight   (bool on) { turnRight_    = on; }

    // Rotate facing by an offset (used by first-person mouse look),
    // so WASD always moves in the direction the player is looking.
    void addYaw(float deg) {
        yaw_ += deg;
        while (yaw_ >  180.f) yaw_ -= 360.f;
        while (yaw_ < -180.f) yaw_ += 360.f;
    }

    // ── Getters ──────────────────────────────────────────────
    Vec3  getPosition () const { return position_; }
    float getYaw      () const { return yaw_;       }
    float getRadius   () const { return radius_;    }
    float getRollAngle() const { return rollAngle_; }
    bool  isAlive     () const { return alive_;     }
    bool  isMoving    () const {
        return moveForward_ || moveBack_ || moveLeft_ || moveRight_;
    }

    // ── Scene reference for wall collision ───────────────────
    void setScene(Scene* s) { scene_ = s; }

    void kill() { alive_ = false; }

private:
    Vec3  position_;
    float yaw_;          // facing direction in degrees (Y-axis rotation)
    float speed_;        // world units per second
    float turnSpeed_;    // degrees per second
    float radius_;       // bounding sphere radius
    float rollAngle_;    // animation: accumulates as player moves

    // Current horizontal velocity (smoothly eased toward the target speed)
    float velX_, velZ_;

    bool  moveForward_, moveBack_, moveLeft_, moveRight_;
    bool  turnLeft_,    turnRight_;
    bool  alive_;

    Scene* scene_;       // borrowed pointer (not owned)

    // Attempt to move by delta; apply axis-separated collision
    void tryMove(float dx, float dz);
};
