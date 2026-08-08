/*
 * Transform.cpp
 * Implementation of Mat4 transformation matrices.
 * All matrices follow OpenGL column-major convention.
 */

#include "Transform.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <cstring>

// ── Default constructor: identity matrix ─────────────────────────────────────
Mat4::Mat4() {
    memset(m, 0, sizeof(m));
    // Diagonal = 1
    m[0] = m[5] = m[10] = m[15] = 1.f;
}

Mat4 Mat4::identity() { return Mat4(); }

// ── Translation ───────────────────────────────────────────────────────────────
Mat4 Mat4::translation(float tx, float ty, float tz) {
    Mat4 t;
    // Column-major: column 3 holds translation
    t.m[12] = tx;
    t.m[13] = ty;
    t.m[14] = tz;
    return t;
}

// ── Rotation around X ─────────────────────────────────────────────────────────
Mat4 Mat4::rotationX(float angleDeg) {
    float r = degToRad(angleDeg);
    float c = std::cos(r), s = std::sin(r);
    Mat4 rx;
    //  column 1          column 2
    rx.m[5]  =  c;   rx.m[9]  = -s;
    rx.m[6]  =  s;   rx.m[10] =  c;
    return rx;
}

// ── Rotation around Y ─────────────────────────────────────────────────────────
Mat4 Mat4::rotationY(float angleDeg) {
    float r = degToRad(angleDeg);
    float c = std::cos(r), s = std::sin(r);
    Mat4 ry;
    //  column 0           column 2
    ry.m[0]  =  c;   ry.m[8]  =  s;
    ry.m[2]  = -s;   ry.m[10] =  c;
    return ry;
}

// ── Rotation around Z ─────────────────────────────────────────────────────────
Mat4 Mat4::rotationZ(float angleDeg) {
    float r = degToRad(angleDeg);
    float c = std::cos(r), s = std::sin(r);
    Mat4 rz;
    //  column 0           column 1
    rz.m[0]  =  c;   rz.m[4]  = -s;
    rz.m[1]  =  s;   rz.m[5]  =  c;
    return rz;
}

// ── Rotation around arbitrary axis (Rodrigues' formula) ───────────────────────
/*
 * Given unit axis n = (nx,ny,nz) and angle θ:
 *   R = cos(θ)·I + (1-cos(θ))·n⊗n + sin(θ)·N×
 * where N× is the skew-symmetric cross-product matrix.
 */
Mat4 Mat4::rotation(float angleDeg, float rx, float ry, float rz) {
    float r   = degToRad(angleDeg);
    float c   = std::cos(r), s = std::sin(r), t = 1.f - c;
    // Normalise axis vector
    float len = std::sqrt(rx*rx + ry*ry + rz*rz);
    if (len < 1e-6f) return identity();
    rx /= len; ry /= len; rz /= len;

    Mat4 rot;
    // Column 0
    rot.m[0] = t*rx*rx + c;      rot.m[1] = t*rx*ry + s*rz;   rot.m[2] = t*rx*rz - s*ry;
    // Column 1
    rot.m[4] = t*rx*ry - s*rz;   rot.m[5] = t*ry*ry + c;      rot.m[6] = t*ry*rz + s*rx;
    // Column 2
    rot.m[8] = t*rx*rz + s*ry;   rot.m[9] = t*ry*rz - s*rx;   rot.m[10]= t*rz*rz + c;
    return rot;
}

// ── Scaling ───────────────────────────────────────────────────────────────────
Mat4 Mat4::scaling(float sx, float sy, float sz) {
    Mat4 s;
    s.m[0]  = sx;
    s.m[5]  = sy;
    s.m[10] = sz;
    return s;
}

// ── Matrix multiplication (C = A * B) ────────────────────────────────────────
Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 res;
    memset(res.m, 0, sizeof(res.m));
    // res[col][row] = sum_k A[k][row] * B[col][k]
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            for (int k = 0; k < 4; k++) {
                res.m[col*4 + row] += m[k*4 + row] * o.m[col*4 + k];
            }
        }
    }
    return res;
}

// ── Push onto OpenGL matrix stack ────────────────────────────────────────────
void Mat4::apply() const {
    glMultMatrixf(m);
}

// ── Debug print ───────────────────────────────────────────────────────────────
void Mat4::print() const {
    for (int row = 0; row < 4; row++) {
        printf("[%7.3f %7.3f %7.3f %7.3f]\n",
               m[row], m[4+row], m[8+row], m[12+row]);
    }
}
