#pragma once
/*
 * Camera.h
 * ============================================================
 * Implements three camera modes:
 *
 *  FIRST_PERSON  – eye at the player's head, looks in movement
 *                  direction (mouse drag rotates view left/right)
 *
 *  THIRD_PERSON  – eye orbits behind/above the player; mouse
 *                  drag changes the orbit yaw and pitch angles
 *
 *  TOP_DOWN      – orthographic bird's-eye view (no perspective
 *                  foreshortening) for a minimap-style layout
 *
 * From Lecture 001/002:
 *   View transform  → lookAt(eye, center, up)
 *   Perspective     → objects shrink with distance (natural vision)
 *   Orthographic    → parallel projection, no foreshortening
 * ============================================================
 */

#include "Transform.h"

// ── Camera mode enum ──────────────────────────────────────────
enum class CameraMode {
    FIRST_PERSON,
    THIRD_PERSON,
    TOP_DOWN
};

class Camera {
public:
    Camera();

    // ── Projection parameters ─────────────────────────────────
    /*
     * Perspective projection (Lecture 001):
     *  Maps the view frustum (defined by fov, aspect, near, far)
     *  to NDC [-1,1].  Creates the foreshortening effect.
     */
    void setPerspective(float fovDeg, float aspect, float nearZ, float farZ);

    /*
     * Orthographic projection (Lecture 001):
     *  Maps a rectangular box to NDC.  No depth-based shrinking.
     *  Used for TOP_DOWN mode.
     */
    void setOrthographic(float left, float right, float bottom, float top,
                         float nearZ, float farZ);

    // ── Mode switching ────────────────────────────────────────
    void       setMode (CameraMode mode);
    CameraMode getMode () const { return mode_; }
    void       cycleMode();          // FIRST → THIRD → TOP_DOWN → FIRST

    // ── Update: recompute eye/center from player state ────────
    /*
     * playerYaw is the facing angle of the player in degrees.
     * playerMoving: when true the third-person camera keeps its current
     * orientation (so camera-relative movement stays in a straight line)
     * and only auto-follows behind the player when standing still.
     * Call this once per frame before applyView().
     */
    void update(const Vec3& playerPos, float playerYaw, float deltaTime,
                bool playerMoving);

    // ── Apply to OpenGL ───────────────────────────────────────
    void applyProjection(int windowW, int windowH) const;
    void applyView()                               const;

    // ── Mouse interaction ─────────────────────────────────────
    /*
     * Call with dx,dy pixel delta when mouse is dragged.
     * In THIRD_PERSON: orbits around player.
     * In FIRST_PERSON: addPitch handles vertical look (horizontal
     * look turns the player via Player::addYaw in Game).
     */
    void onMouseDrag(int dx, int dy);

    // First-person vertical look angle (degrees, clamped to ±89°)
    void addPitch(float deg);

    // Call when the mouse button is released so the third-person camera
    // resumes auto-following behind the player.
    void endMouseOrbit();

    // Zoom the third-person orbit distance (mouse wheel); clamps range.
    void zoom(float delta);

    // ── Getters / setters ─────────────────────────────────────
    Vec3 getEye()     const { return eye_; }
    Vec3 getCenter()  const { return center_; }
    void setEye(const Vec3& e) { eye_ = e; }
    // Set eye position AND sync the smoothing state, so wall-collision
    // clamping doesn't get undone by the smooth-follow lerp on the next frame.
    void snapEye(const Vec3& e) { eye_ = e; smoothEye_ = e; }

    // Horizontal forward/right vectors (for movement in FP mode)
    Vec3 getForward() const;
    Vec3 getRight()   const;

private:
    CameraMode mode_;

    // Perspective params
    float fov_, aspect_, nearZ_, farZ_;

    // Orthographic params
    float orthoNear_, orthoFar_;

    // Current view vectors
    Vec3 eye_, center_, up_;

    // Third-person orbit angles
    float orbitYaw_;    // degrees, horizontal around player
    float orbitPitch_;  // degrees, elevation above player
    float orbitDist_;   // world units

    // True while the user is dragging the orbit camera (disables auto-follow)
    bool  orbitActive_;

    // First-person extra pitch from mouse look
    float fpPitch_;

    // Smooth-follow state (prevents jarring camera cuts)
    Vec3  smoothEye_, smoothCenter_;
    float smoothSpeed_;

    // Internal: compute target eye/center then smooth-lerp to them
    void computeTargets(const Vec3& playerPos, float playerYaw,
                        Vec3& outEye, Vec3& outCenter, Vec3& outUp) const;
};
