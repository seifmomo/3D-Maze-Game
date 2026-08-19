/*
 * Camera.cpp
 * ============================================================
 * Three-mode camera system:
 *
 *  FIRST_PERSON  – immersive; player controls look direction
 *  THIRD_PERSON  – orbital follow cam; mouse drag to orbit
 *  TOP_DOWN      – orthographic overhead; no foreshortening
 *
 * The view transform is implemented via gluLookAt, which builds
 * the classic lookAt matrix:
 *   - forward f = normalize(center - eye)
 *   - right   r = normalize(f × up)
 *   - true-up u = r × f
 * Then arranges r, u, -f as rows of the rotation part.
 * ============================================================
 */

#include "Camera.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <algorithm>

// ── Constructor ───────────────────────────────────────────────────────────────
Camera::Camera()
    : mode_(CameraMode::THIRD_PERSON)
    , fov_(60.f), aspect_(4.f/3.f), nearZ_(0.1f), farZ_(200.f)
    , orthoNear_(-100.f), orthoFar_(100.f)
    , eye_(0,6,-6), center_(0,0,0), up_(0,1,0)
    , orbitYaw_(180.f)    // start behind player (facing its back)
    , orbitPitch_(28.f)   // elevated 28° above horizon
    , orbitDist_(6.f)
    , orbitActive_(false)
    , fpPitch_(0.f)
    , smoothEye_(0,6,-6), smoothCenter_(0,0,0)
    , smoothSpeed_(10.f)
{}

// ── Projection setters ────────────────────────────────────────────────────────
void Camera::setPerspective(float fov, float aspect, float nearZ, float farZ) {
    fov_    = fov;
    aspect_ = aspect;
    nearZ_  = nearZ;
    farZ_   = farZ;
}

void Camera::setOrthographicRange(float nearZ, float farZ) {
    orthoNear_ = nearZ;
    orthoFar_  = farZ;
}

// ── Mode switching ────────────────────────────────────────────────────────────
void Camera::setMode(CameraMode mode) {
    mode_ = mode;
    // Reset smooth state to current so there's no "teleport" glitch
    smoothEye_    = eye_;
    smoothCenter_ = center_;
}

void Camera::cycleMode() {
    switch (mode_) {
        case CameraMode::FIRST_PERSON: setMode(CameraMode::THIRD_PERSON); break;
        case CameraMode::THIRD_PERSON: setMode(CameraMode::TOP_DOWN);     break;
        case CameraMode::TOP_DOWN:     setMode(CameraMode::FIRST_PERSON); break;
    }
}

// ── Mouse drag ────────────────────────────────────────────────────────────────
void Camera::onMouseDrag(int dx, int dy) {
    if (mode_ == CameraMode::THIRD_PERSON) {
        orbitActive_ = true;   // user controls the orbit; disable auto-follow
        orbitYaw_   += dx * 0.4f;
        orbitPitch_ -= dy * 0.4f;
        // Clamp pitch so we never flip over the top or look below floor
        orbitPitch_ = std::max(5.f, std::min(85.f, orbitPitch_));
    }
}

void Camera::endMouseOrbit() {
    orbitActive_ = false;
}

void Camera::zoom(float delta) {
    if (mode_ != CameraMode::THIRD_PERSON) return;
    orbitDist_ += delta;
    orbitDist_ = std::max(2.5f, std::min(14.f, orbitDist_));
}

void Camera::addPitch(float deg) {
    fpPitch_ += deg;
    // Clamp so we never look straight up/down (avoids camera gimbal flip)
    fpPitch_ = std::max(-89.f, std::min(89.f, fpPitch_));
}

// ── Compute target eye/center/up ──────────────────────────────────────────────
void Camera::computeTargets(const Vec3& pPos, float pYaw,
                             Vec3& outEye, Vec3& outCenter, Vec3& outUp) const {
    switch (mode_) {

    // ── FIRST PERSON ─────────────────────────────────────────────────────────
    // Eye is placed at the player's head; look direction follows the player's
    // facing yaw plus any mouse-look pitch (up/down).
    case CameraMode::FIRST_PERSON: {
        float totalYawRad = degToRad(pYaw);
        float pitchRad    = degToRad(fpPitch_);
        float cosPitch    = std::cos(pitchRad);
        outEye    = pPos + Vec3(0, 0.55f, 0);   // eye at head height
        outCenter = outEye + Vec3(
            -std::sin(totalYawRad) * cosPitch,
              std::sin(pitchRad),
            -std::cos(totalYawRad) * cosPitch
        );
        outUp = {0, 1, 0};
        break;
    }

    // ── THIRD PERSON ─────────────────────────────────────────────────────────
    // Eye orbits around the player at a configurable yaw+pitch+distance.
    // orbitYaw is independent of player's facing so mouse drag truly orbits.
    case CameraMode::THIRD_PERSON: {
        float orbitYawRad   = degToRad(orbitYaw_);
        float orbitPitchRad = degToRad(orbitPitch_);
        float offX = orbitDist_ * std::cos(orbitPitchRad) * std::sin(orbitYawRad);
        float offY = orbitDist_ * std::sin(orbitPitchRad);
        float offZ = orbitDist_ * std::cos(orbitPitchRad) * std::cos(orbitYawRad);
        outEye    = pPos + Vec3(offX, offY, offZ);
        outCenter = pPos + Vec3(0, 0.4f, 0);   // look slightly above player origin
        outUp     = {0, 1, 0};
        break;
    }

    // ── TOP DOWN (orthographic) ───────────────────────────────────────────────
    // Eye looks straight down from high altitude.
    // 'up' vector is aligned with -Z (screen top = world north).
    case CameraMode::TOP_DOWN: {
        outEye    = pPos + Vec3(0, 22.f, 0.01f);
        outCenter = pPos;
        outUp     = {0, 0, -1};
        break;
    }
    }
}

// ── Update: smooth-interpolate towards target ─────────────────────────────────
void Camera::update(const Vec3& playerPos, float playerYaw, float deltaTime,
                    bool playerMoving) {
    // Third-person auto-follow: when the user is NOT dragging AND the player is
    // NOT moving, slowly swing the orbit around to sit behind the player's
    // facing. Skipped while moving so the camera's forward direction (the
    // movement reference) stays fixed and the ball travels in a straight line.
    if (mode_ == CameraMode::THIRD_PERSON && !orbitActive_ && !playerMoving) {
        float targetYaw = playerYaw + 180.f;
        float diff = targetYaw - orbitYaw_;
        while (diff >  180.f) diff -= 360.f;
        while (diff < -180.f) diff += 360.f;
        orbitYaw_ += diff * std::min(1.f, 4.f * deltaTime);
    }

    Vec3 tEye, tCenter, tUp;
    computeTargets(playerPos, playerYaw, tEye, tCenter, tUp);

    // In FIRST_PERSON we want instant response; other modes smooth-follow.
    float t = (mode_ == CameraMode::FIRST_PERSON)
               ? 1.f
               : std::min(1.f, smoothSpeed_ * deltaTime);

    auto lerp3 = [](Vec3 a, Vec3 b, float t) {
        return a + (b - a) * t;
    };

    smoothEye_    = lerp3(smoothEye_,    tEye,    t);
    smoothCenter_ = lerp3(smoothCenter_, tCenter, t);
    up_           = tUp;

    eye_    = smoothEye_;
    center_ = smoothCenter_;
}

// ── Apply projection matrix to OpenGL ─────────────────────────────────────────
void Camera::applyProjection(int windowW, int windowH) const {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (mode_ == CameraMode::TOP_DOWN) {
        /*
         * Orthographic projection — no foreshortening.
         * Size the view box to fit the maze footprint (30 world units)
         * with a small margin; expand the vertical half-extent so the
         * whole maze is always visible regardless of window aspect.
         */
        float half = 15.5f;
        float halfH = half * (float)windowH / (float)(windowW > 0 ? windowW : 1);
        if (halfH < half) halfH = half;
        glOrtho(-half, half, -halfH, halfH, orthoNear_, orthoFar_);
    } else {
        /*
         * Perspective projection — implements the pinhole camera model.
         * gluPerspective internally builds:
         *   [ f/a   0    0        0      ]
         *   [  0    f    0        0      ]
         *   [  0    0  (n+f)/(n-f) 2nf/(n-f) ]
         *   [  0    0   -1        0      ]
         * where f = cot(fov/2), a = aspect ratio
         */
        float asp = (windowH > 0) ? (float)windowW / (float)windowH : aspect_;
        gluPerspective(fov_, asp, nearZ_, farZ_);
    }

    glMatrixMode(GL_MODELVIEW);
}

// ── Apply view matrix (lookAt) ────────────────────────────────────────────────
void Camera::applyView() const {
    gluLookAt(
        eye_.x,    eye_.y,    eye_.z,
        center_.x, center_.y, center_.z,
        up_.x,     up_.y,     up_.z
    );
}

// ── Horizontal forward/right (for player movement input) ─────────────────────
Vec3 Camera::getForward() const {
    Vec3 f = (center_ - eye_);
    f.y = 0;
    return f.normalize();
}

Vec3 Camera::getRight() const {
    Vec3 f = getForward();
    return f.cross(Vec3(0,1,0)).normalize();
}
