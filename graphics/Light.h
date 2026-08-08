#pragma once
/*
 * Light.h
 * ============================================================
 * Implements the Blinn-Phong illumination model with multiple
 * light sources through OpenGL's fixed-function lighting API.
 *
 * From Lecture 001 – Illumination Models:
 *
 *   I = Ka·Ia  +  Kd·Id·max(0, N·L)  +  Ks·Is·max(0, N·H)^n
 *
 * Where:
 *   Ia, Id, Is  = ambient / diffuse / specular light intensity
 *   Ka, Kd, Ks  = material ambient / diffuse / specular reflectance
 *   N           = surface normal (unit vector, interpolated per pixel)
 *   L           = unit vector from surface point to light source
 *   H           = Blinn's half-vector = normalize(L + V)
 *                 (V = view direction; avoids expensive reflect())
 *   n           = shininess exponent (higher = tighter highlight)
 *
 * Two lights are used:
 *   GL_LIGHT0 – directional "sun" (infinite distance, w=0)
 *   GL_LIGHT1 – point light near player (finite position, w=1)
 *
 * The w component distinguishes light types in OpenGL.
 * ============================================================
 */

#include "Transform.h"

struct LightColor {
    float r, g, b, a;
    LightColor(float r=1,float g=1,float b=1,float a=1):r(r),g(g),b(b),a(a){}
};

enum class LightType {
    DIRECTIONAL,  // w=0 in position array → infinite distance
    POINT         // w=1 in position array → finite world position
};

// ─────────────────────────────────────────────────────────────
//  Light class
// ─────────────────────────────────────────────────────────────
class Light {
public:
    /*
     * glLightId must be GL_LIGHT0 .. GL_LIGHT7.
     * OpenGL guarantees at least 8 simultaneous lights.
     */
    Light(int glLightId, LightType type);

    // ── Configure colour components ───────────────────────────
    void setAmbient (const LightColor& c);
    void setDiffuse (const LightColor& c);
    void setSpecular(const LightColor& c);

    // ── Configure position / direction ────────────────────────
    void setPosition (const Vec3& pos);   // world position (POINT)
    void setDirection(const Vec3& dir);   // light direction (DIRECTIONAL)

    // ── Attenuation for point lights ──────────────────────────
    // Iatt = 1 / (kc + kl·d + kq·d²)
    void setAttenuation(float constant, float linear, float quadratic);

    // ── Enable / disable ──────────────────────────────────────
    void enable();
    void disable();
    bool isEnabled() const { return enabled_; }

    // ── Apply to OpenGL (call each frame in modelview space) ──
    void apply() const;

    // ── Move light by a delta vector ──────────────────────────
    void move(const Vec3& delta);
    Vec3 getPosition() const { return position_; }

private:
    int        glLightId_;
    LightType  type_;
    Vec3       position_;
    LightColor ambient_, diffuse_, specular_;
    float      constAtten_, linearAtten_, quadAtten_;
    bool       enabled_;
};

// ─────────────────────────────────────────────────────────────
//  Material helpers (set per-surface Phong coefficients)
// ─────────────────────────────────────────────────────────────
/*
 * glMaterial* sets Ka, Kd, Ks, and shininess (n) for the current
 * surface.  OpenGL uses these in the lighting equation above.
 */
void setMaterial(const LightColor& ambient,
                 const LightColor& diffuse,
                 const LightColor& specular,
                 float             shininess);

// Pre-defined physically-based material presets
void materialGold();
void materialSilver();
void materialRuby();
void materialEmerald();
void materialWall();
void materialFloor();
void materialPlayer();
void materialGuard();
