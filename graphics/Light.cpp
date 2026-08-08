/*
 * Light.cpp
 * Blinn-Phong lighting via OpenGL fixed-function API.
 * See Light.h for the full illumination model description.
 */

#include "Light.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>

// ── Constructor ───────────────────────────────────────────────────────────────
Light::Light(int glLightId, LightType type)
    : glLightId_(glLightId)
    , type_(type)
    , position_(0, 10, 0)
    , ambient_ (0.1f, 0.1f, 0.1f, 1.f)
    , diffuse_ (1.f,  1.f,  1.f,  1.f)
    , specular_(1.f,  1.f,  1.f,  1.f)
    , constAtten_(1.f), linearAtten_(0.f), quadAtten_(0.f)
    , enabled_(false)
{}

// ── Setters ───────────────────────────────────────────────────────────────────
void Light::setAmbient (const LightColor& c) { ambient_  = c; }
void Light::setDiffuse (const LightColor& c) { diffuse_  = c; }
void Light::setSpecular(const LightColor& c) { specular_ = c; }
void Light::setPosition (const Vec3& pos)    { position_ = pos; }
void Light::setDirection(const Vec3& dir)    { position_ = dir; } // stored in position for directional
void Light::setAttenuation(float c, float l, float q) {
    constAtten_ = c; linearAtten_ = l; quadAtten_ = q;
}

// ── Enable / disable ──────────────────────────────────────────────────────────
void Light::enable()  { enabled_ = true;  glEnable((unsigned int)glLightId_);  }
void Light::disable() { enabled_ = false; glDisable((unsigned int)glLightId_); }

// ── Move ──────────────────────────────────────────────────────────────────────
void Light::move(const Vec3& delta) { position_ += delta; }

// ── Apply to OpenGL ───────────────────────────────────────────────────────────
void Light::apply() const {
    if (!enabled_) return;

    unsigned int id = (unsigned int)glLightId_;

    float amb[4] = { ambient_.r,  ambient_.g,  ambient_.b,  ambient_.a };
    float dif[4] = { diffuse_.r,  diffuse_.g,  diffuse_.b,  diffuse_.a };
    float spe[4] = { specular_.r, specular_.g, specular_.b, specular_.a };

    glLightfv(id, GL_AMBIENT,  amb);
    glLightfv(id, GL_DIFFUSE,  dif);
    glLightfv(id, GL_SPECULAR, spe);

    /*
     * w = 0 → directional light (direction is position_.xyz)
     * w = 1 → point light at world position position_.xyz
     * OpenGL transforms this position by the current modelview matrix,
     * so we call apply() AFTER setting up the view transform.
     */
    float w = (type_ == LightType::DIRECTIONAL) ? 0.f : 1.f;
    float pos[4] = { position_.x, position_.y, position_.z, w };
    glLightfv(id, GL_POSITION, pos);

    if (type_ == LightType::POINT) {
        // Attenuation: Iatt = 1 / (kc + kl*d + kq*d²)
        glLightf(id, GL_CONSTANT_ATTENUATION,  constAtten_);
        glLightf(id, GL_LINEAR_ATTENUATION,    linearAtten_);
        glLightf(id, GL_QUADRATIC_ATTENUATION, quadAtten_);
    }
}

// ── Generic material setter ───────────────────────────────────────────────────
void setMaterial(const LightColor& amb, const LightColor& dif,
                 const LightColor& spe, float shininess) {
    float a[4] = {amb.r, amb.g, amb.b, amb.a};
    float d[4] = {dif.r, dif.g, dif.b, dif.a};
    float s[4] = {spe.r, spe.g, spe.b, spe.a};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

// ── Pre-defined material presets (physically-based approximations) ─────────────
void materialGold() {
    setMaterial(
        {0.25f, 0.20f, 0.07f, 1.f},
        {0.75f, 0.61f, 0.23f, 1.f},
        {0.63f, 0.56f, 0.37f, 1.f},
        51.2f
    );
}
void materialSilver() {
    setMaterial(
        {0.19f, 0.19f, 0.19f, 1.f},
        {0.51f, 0.51f, 0.51f, 1.f},
        {0.51f, 0.51f, 0.51f, 1.f},
        51.2f
    );
}
void materialRuby() {
    setMaterial(
        {0.17f, 0.01f, 0.01f, 1.f},
        {0.61f, 0.04f, 0.04f, 1.f},
        {0.73f, 0.63f, 0.63f, 1.f},
        76.8f
    );
}
void materialEmerald() {
    setMaterial(
        {0.02f, 0.17f, 0.02f, 1.f},
        {0.08f, 0.61f, 0.08f, 1.f},
        {0.63f, 0.73f, 0.63f, 1.f},
        76.8f
    );
}
void materialWall() {
    setMaterial(
        {0.08f, 0.08f, 0.14f, 1.f},
        {0.25f, 0.25f, 0.45f, 1.f},
        {0.12f, 0.12f, 0.22f, 1.f},
        12.f
    );
}
void materialFloor() {
    setMaterial(
        {0.04f, 0.09f, 0.04f, 1.f},
        {0.12f, 0.30f, 0.12f, 1.f},
        {0.04f, 0.06f, 0.04f, 1.f},
        5.f
    );
}
void materialPlayer() {
    setMaterial(
        {0.05f, 0.25f, 0.05f, 1.f},
        {0.10f, 0.80f, 0.10f, 1.f},
        {0.40f, 0.90f, 0.40f, 1.f},
        64.f
    );
}
void materialGuard() {
    setMaterial(
        {0.25f, 0.03f, 0.03f, 1.f},
        {0.80f, 0.08f, 0.08f, 1.f},
        {0.90f, 0.50f, 0.50f, 1.f},
        32.f
    );
}
