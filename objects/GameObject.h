#pragma once
/*
 * GameObject.h
 * ============================================================
 * Base class for all renderable game entities.
 * Stores position, rotation (Euler Y/X/Z), and scale.
 *
 * HIERARCHICAL MODELLING (Lecture 006):
 *   Parent transforms are applied first; children inherit them.
 *   This is implemented by nesting glPushMatrix/glPopMatrix calls.
 *   Example: player body → eye (child) rendered in body-local space.
 *
 * Transform order: T * Ry * Rx * Rz * S
 *   (translation outermost, then rotation, then scale innermost)
 * ============================================================
 */

#include "../graphics/Transform.h"
#include <vector>

class GameObject {
public:
    // ── World-space transform ─────────────────────────────────
    Vec3  position;
    float rotY;       // degrees, rotation around world Y (yaw/facing)
    float rotX;       // degrees, rotation around X (pitch)
    float rotZ;       // degrees, rotation around Z (roll)
    Vec3  scale;
    bool  active;

    // Bounding sphere for collision detection
    float boundRadius;

    // ── Constructor / destructor ──────────────────────────────
    GameObject();
    virtual ~GameObject() {}

    // ── Hierarchy management ──────────────────────────────────
    /*
     * addChild: registers obj as a child of this node.
     * During drawAll(), children are drawn inside the parent's
     * push/pop matrix block, so they inherit the parent transform.
     */
    void addChild(GameObject* child);

    // ── Draw dispatch ─────────────────────────────────────────
    /*
     * drawAll() pushes this object's transform, calls draw(),
     * then recursively calls drawAll() for each child, and
     * finally pops the matrix.  Do NOT override drawAll().
     */
    void drawAll();

    // ── Virtual interface ─────────────────────────────────────
    virtual void draw()           {}   // draw this node's geometry
    virtual void update(float dt) {}   // update per-frame logic

    // ── Collision helpers ─────────────────────────────────────
    bool collidesWithSphere(const Vec3& otherPos, float otherRadius) const;

protected:
    /*
     * pushTransform: glPushMatrix then apply T, Ry, Rx, Rz, S.
     * popTransform:  glPopMatrix.
     * These are separated so subclasses can inject extra transforms
     * between them if needed.
     */
    void pushTransform() const;
    void popTransform()  const;

private:
    std::vector<GameObject*> children_;
};
