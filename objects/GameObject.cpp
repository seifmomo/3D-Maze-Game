/*
 * GameObject.cpp
 * Hierarchical transform implementation.
 */

#include "GameObject.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>

// ── Constructor ───────────────────────────────────────────────────────────────
GameObject::GameObject()
    : position(0,0,0)
    , rotY(0), rotX(0), rotZ(0)
    , scale(1,1,1)
    , active(true)
    , boundRadius(0.5f)
{}

// ── Hierarchy ─────────────────────────────────────────────────────────────────
void GameObject::addChild(GameObject* child) {
    children_.push_back(child);
}

// ── Push transform ────────────────────────────────────────────────────────────
/*
 * Transform order (applied right-to-left, as is standard):
 *   1. Scale   (innermost – scales geometry in local space)
 *   2. Rotate  (then rotate scaled geometry)
 *   3. Translate (outermost – moves to world position)
 *
 * Equivalent to: glTranslate * glRotateY * glRotateX * glRotateZ * glScale
 */
void GameObject::pushTransform() const {
    glPushMatrix();
    Mat4::translation(position.x, position.y, position.z).apply();
    if (rotY != 0.f) Mat4::rotationY(rotY).apply();
    if (rotX != 0.f) Mat4::rotationX(rotX).apply();
    if (rotZ != 0.f) Mat4::rotationZ(rotZ).apply();
    if (scale.x != 1.f || scale.y != 1.f || scale.z != 1.f)
        Mat4::scaling(scale.x, scale.y, scale.z).apply();
}

void GameObject::popTransform() const {
    glPopMatrix();
}

// ── Draw all (self + children) ────────────────────────────────────────────────
void GameObject::drawAll() {
    if (!active) return;
    pushTransform();
    draw();                             // draw this node
    for (GameObject* c : children_)
        c->drawAll();                   // draw children in parent-local space
    popTransform();
}

// ── Bounding sphere collision ─────────────────────────────────────────────────
bool GameObject::collidesWithSphere(const Vec3& otherPos, float otherRadius) const {
    Vec3 diff = position - otherPos;
    float dist2  = diff.dot(diff);
    float radSum = boundRadius + otherRadius;
    return dist2 < radSum * radSum;
}
