/*
 * Player.cpp
 * ============================================================
 * Frame-based player movement with collision detection.
 *
 * Movement derivation (Lecture 006):
 *   Forward vector from yaw angle θ:
 *     fwd = (-sin(θ), 0, -cos(θ))     [right-handed, Y-up, -Z forward]
 *   Right vector (perpendicular):
 *     right = (cos(θ), 0, -sin(θ))    [verified: fwd×up = right]
 *
 * Delta-time usage:
 *   distance_per_frame = speed (m/s) × deltaTime (s)
 *   This ensures the player moves at the same real-world speed
 *   regardless of frame rate.
 * ============================================================
 */

#include "Player.h"
#include "Scene.h"
#include "../objects/ComplexObject.h"
#include "../graphics/Transform.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ── Constructor ───────────────────────────────────────────────────────────────
Player::Player()
    : position_(0, 0.5f, 0)
    , yaw_(0)
    , speed_(5.4f)
    , turnSpeed_(130.f)
    , radius_(0.32f)
    , rollAngle_(0)
    , velX_(0), velZ_(0)
    , moveForward_(false), moveBack_(false)
    , moveLeft_(false),    moveRight_(false)
    , turnLeft_(false),    turnRight_(false)
    , alive_(true)
    , scene_(nullptr)
{}

// ── Init / respawn ────────────────────────────────────────────────────────────
void Player::init(const Vec3& startPos) {
    position_  = startPos;
    yaw_       = 0;
    rollAngle_ = 0;
    velX_ = velZ_ = 0;
    alive_     = true;
    // Reset all input flags
    moveForward_ = moveBack_ = moveLeft_ = moveRight_ = false;
    turnLeft_    = turnRight_ = false;
}

void Player::respawn(const Vec3& startPos) {
    init(startPos);
}

// ── Update (called every frame) ───────────────────────────────────────────────
void Player::update(float dt, const Vec3& moveFwd, const Vec3& moveRight) {
    if (!alive_) return;

    // ── 1. Optional turning (Q/E) ──────────────────────────────
    if (turnLeft_)  yaw_ -= turnSpeed_ * dt;
    if (turnRight_) yaw_ += turnSpeed_ * dt;

    // ── 2. Target velocity from camera-relative input ──────────
    // W/A/S/D are resolved against the camera's forward/right so the ball
    // moves directly in the direction the player sees on screen.
    float tx = 0, tz = 0;
    if (moveForward_)  { tx += moveFwd.x;    tz += moveFwd.z; }
    if (moveBack_)     { tx -= moveFwd.x;    tz -= moveFwd.z; }
    if (moveLeft_)     { tx -= moveRight.x;  tz -= moveRight.z; }
    if (moveRight_)    { tx += moveRight.x;  tz += moveRight.z; }

    float tLen = std::sqrt(tx*tx + tz*tz);
    if (tLen > 0.001f) {
        tx = tx / tLen * speed_;
        tz = tz / tLen * speed_;

        // Face the direction of travel so the ball's eye looks where it rolls.
        yaw_ = radToDeg(std::atan2(-tx / speed_, -tz / speed_));
    }

    // ── 3. Ease current velocity toward the target ─────────────
    // Exponential smoothing gives a short, natural acceleration/stop instead
    // of instantly snapping to full speed (feels weighty but responsive).
    float k = 1.f - std::exp(-9.f * dt);
    velX_ += (tx - velX_) * k;
    velZ_ += (tz - velZ_) * k;

    float prevX = position_.x;
    float prevZ = position_.z;
    tryMove(velX_ * dt, velZ_ * dt);

    // ── 4. Ball roll animation ─────────────────────────────────
    // Arc length = radius × angle → angle = arc / radius
    float distMoved = std::sqrt(
        (position_.x - prevX)*(position_.x - prevX) +
        (position_.z - prevZ)*(position_.z - prevZ)
    );
    rollAngle_ += radToDeg(distMoved / radius_);
}

// ── Axis-separated collision resolution ──────────────────────────────────────
void Player::tryMove(float dx, float dz) {
    if (!scene_) {
        position_.x += dx;
        position_.z += dz;
        return;
    }

    // Try X axis independently
    Vec3 testX = position_ + Vec3(dx, 0, 0);
    if (!scene_->isWall(testX, radius_ * 0.85f)) {
        position_.x = testX.x;
    }

    // Try Z axis independently
    Vec3 testZ = position_ + Vec3(0, 0, dz);
    if (!scene_->isWall(testZ, radius_ * 0.85f)) {
        position_.z = testZ.z;
    }
}

// ── Draw ──────────────────────────────────────────────────────────────────────
void Player::draw() {
    if (!alive_) return;

    glPushMatrix();

    // Translate to world position
    Mat4::translation(position_.x, position_.y, position_.z).apply();

    // Draw the composite player ball (sphere + eye hierarchy).
    // +180° so the eye faces the direction of travel (local +Z after yaw
    // rotation points along the forward vector).
    ComplexObjects::drawPlayerBall(radius_, rollAngle_, yaw_ + 180.f);

    glPopMatrix();
}
