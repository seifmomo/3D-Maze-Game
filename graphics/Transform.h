#pragma once
/*
 * Transform.h
 * ============================================================
 * Implements 3D geometric transformations using 4x4 homogeneous
 * coordinate matrices.  From Lecture 006: Translation, Rotation,
 * Scaling in 3D space.
 *
 * OpenGL stores matrices in COLUMN-MAJOR order:
 *   m[0] m[4] m[8]  m[12]   <- column 0, 1, 2, 3
 *   m[1] m[5] m[9]  m[13]
 *   m[2] m[6] m[10] m[14]
 *   m[3] m[7] m[11] m[15]
 *
 * Homogeneous coords let us express translation as matrix multiply,
 * unifying all affine transforms: T * R * S * vertex
 * ============================================================
 */

#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─────────────────────────────────────────────────────────────
//  Vec3 : 3-component vector
// ─────────────────────────────────────────────────────────────
struct Vec3 {
    float x, y, z;

    Vec3(float x = 0.f, float y = 0.f, float z = 0.f) : x(x), y(y), z(z) {}

    Vec3  operator+(const Vec3& o)  const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3  operator-(const Vec3& o)  const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3  operator*(float s)        const { return {x*s,   y*s,   z*s  }; }
    Vec3  operator-()               const { return {-x,    -y,    -z   }; }
    Vec3& operator+=(const Vec3& o)       { x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator-=(const Vec3& o)       { x-=o.x; y-=o.y; z-=o.z; return *this; }

    float dot  (const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3  cross(const Vec3& o) const {
        return { y*o.z - z*o.y,  z*o.x - x*o.z,  x*o.y - y*o.x };
    }
    float length()    const { return std::sqrt(x*x + y*y + z*z); }
    float lengthSq()  const { return x*x + y*y + z*z; }
    Vec3  normalize() const {
        float l = length();
        return (l > 1e-6f) ? Vec3{x/l, y/l, z/l} : Vec3{0,0,0};
    }
};

// ─────────────────────────────────────────────────────────────
//  Mat4 : 4x4 matrix (column-major, OpenGL convention)
// ─────────────────────────────────────────────────────────────
struct Mat4 {
    float m[16]; // column-major

    // Default: identity
    Mat4();

    // ── Factory methods ──────────────────────────────────────

    static Mat4 identity();

    /*
     * Translation matrix (Lecture 006):
     *  | 1  0  0  tx |
     *  | 0  1  0  ty |
     *  | 0  0  1  tz |
     *  | 0  0  0   1 |
     * In column-major: tx → m[12], ty → m[13], tz → m[14]
     */
    static Mat4 translation(float tx, float ty, float tz);

    /*
     * Rotation around X axis:
     *  | 1   0     0   0 |
     *  | 0  cos  -sin  0 |
     *  | 0  sin   cos  0 |
     *  | 0   0     0   1 |
     */
    static Mat4 rotationX(float angleDeg);

    /*
     * Rotation around Y axis:
     *  | cos   0  sin  0 |
     *  |  0    1   0   0 |
     *  |-sin   0  cos  0 |
     *  |  0    0   0   1 |
     */
    static Mat4 rotationY(float angleDeg);

    /*
     * Rotation around Z axis:
     *  | cos  -sin  0  0 |
     *  | sin   cos  0  0 |
     *  |  0     0   1  0 |
     *  |  0     0   0  1 |
     */
    static Mat4 rotationZ(float angleDeg);

    /*
     * Rotation around arbitrary axis using Rodrigues' formula.
     * The axis (rx,ry,rz) is normalised internally.
     * angle in DEGREES.
     */
    static Mat4 rotation(float angleDeg, float rx, float ry, float rz);

    /*
     * Scaling matrix (Lecture 006):
     *  | sx  0   0   0 |
     *  |  0  sy  0   0 |
     *  |  0   0  sz  0 |
     *  |  0   0   0  1 |
     */
    static Mat4 scaling(float sx, float sy, float sz);

    // Matrix multiplication: combines two transformations (C = A*B)
    Mat4 operator*(const Mat4& o) const;

    // Push this matrix onto OpenGL's current matrix stack
    void apply() const;

    // Debug print
    void print() const;
};

// ── Utility conversions ────────────────────────────────────────
inline float degToRad(float deg) { return deg * (float)M_PI / 180.f; }
inline float radToDeg(float rad) { return rad * 180.f / (float)M_PI; }
