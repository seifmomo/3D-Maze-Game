/*
 * Primitives.cpp
 * ============================================================
 * Immediate-mode OpenGL drawing for primitive 3D shapes.
 * Each function emits vertices + normals so the fixed-function
 * lighting pipeline can shade them correctly.
 *
 * Key principle (Lecture 004):
 *   Normals must point OUTWARD from the surface.
 *   For a unit sphere:  N = vertex position (radius = 1)
 *   For a cube face:    N = constant face normal
 * ============================================================
 */

#include "Primitives.h"
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Primitives {

// ── Cube ──────────────────────────────────────────────────────────────────────
void drawCube(float w, float h, float d) {
    float hw = w * 0.5f;
    float hh = h * 0.5f;
    float hd = d * 0.5f;

    glBegin(GL_QUADS);

    // Front face (+Z normal)
    glNormal3f(0, 0, 1);
    glVertex3f(-hw, -hh,  hd);  glVertex3f( hw, -hh,  hd);
    glVertex3f( hw,  hh,  hd);  glVertex3f(-hw,  hh,  hd);

    // Back face (-Z normal)
    glNormal3f(0, 0, -1);
    glVertex3f( hw, -hh, -hd);  glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw,  hh, -hd);  glVertex3f( hw,  hh, -hd);

    // Left face (-X normal)
    glNormal3f(-1, 0, 0);
    glVertex3f(-hw, -hh, -hd);  glVertex3f(-hw, -hh,  hd);
    glVertex3f(-hw,  hh,  hd);  glVertex3f(-hw,  hh, -hd);

    // Right face (+X normal)
    glNormal3f(1, 0, 0);
    glVertex3f( hw, -hh,  hd);  glVertex3f( hw, -hh, -hd);
    glVertex3f( hw,  hh, -hd);  glVertex3f( hw,  hh,  hd);

    // Top face (+Y normal)
    glNormal3f(0, 1, 0);
    glVertex3f(-hw,  hh,  hd);  glVertex3f( hw,  hh,  hd);
    glVertex3f( hw,  hh, -hd);  glVertex3f(-hw,  hh, -hd);

    // Bottom face (-Y normal)
    glNormal3f(0, -1, 0);
    glVertex3f(-hw, -hh, -hd);  glVertex3f( hw, -hh, -hd);
    glVertex3f( hw, -hh,  hd);  glVertex3f(-hw, -hh,  hd);

    glEnd();
}

// ── Sphere ────────────────────────────────────────────────────────────────────
/*
 * UV sphere: parameterised by angles phi (latitude) and theta (longitude).
 *   x = r * cos(phi) * cos(theta)
 *   y = r * sin(phi)
 *   z = r * cos(phi) * sin(theta)
 * Normal at any surface point = position / radius (outward radial).
 */
void drawSphere(float radius, int slices, int stacks) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);   // per-vertex smooth normals
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
}

// ── Cylinder ──────────────────────────────────────────────────────────────────
void drawCylinder(float radius, float height, int slices) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);

    glPushMatrix();
    // GLU cylinder goes along +Z; we want along +Y, centred.
    glTranslatef(0, -height * 0.5f, 0);
    glRotatef(-90.f, 1, 0, 0);   // rotate so Z→Y

    gluCylinder(q, radius, radius, height, slices, 1);

    // Bottom cap (disk at z=0, normal -Y after our rotation)
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    gluDisk(q, 0, radius, slices, 1);
    glPopMatrix();

    // Top cap (disk at z=height)
    glPushMatrix();
    glTranslatef(0, 0, height);
    gluDisk(q, 0, radius, slices, 1);
    glPopMatrix();

    glPopMatrix();
    gluDeleteQuadric(q);
}

// ── Cone ──────────────────────────────────────────────────────────────────────
void drawCone(float baseRadius, float height, int slices) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);

    glPushMatrix();
    glTranslatef(0, -height * 0.5f, 0);
    glRotatef(-90.f, 1, 0, 0);

    gluCylinder(q, baseRadius, 0.f, height, slices, 1);  // cone = cylinder with apex radius=0

    // Base disk
    glRotatef(180, 1, 0, 0);
    gluDisk(q, 0, baseRadius, slices, 1);

    glPopMatrix();
    gluDeleteQuadric(q);
}

// ── Pyramid ───────────────────────────────────────────────────────────────────
/*
 * Square pyramid: apex at +Y, square base at -Y.
 * Normals computed analytically for each triangular face.
 */
void drawPyramid(float base, float height) {
    float h2 = height * 0.5f;
    float b2 = base   * 0.5f;

    // Approximate side-face normal magnitude
    float nXZ = height;
    float nY  = b2;
    float nLen = std::sqrt(nXZ*nXZ + nY*nY);
    nXZ /= nLen; nY /= nLen;

    glBegin(GL_TRIANGLES);

    // Front face (+Z)
    glNormal3f(0,   nY,  nXZ);
    glVertex3f(-b2, -h2,  b2);
    glVertex3f( b2, -h2,  b2);
    glVertex3f(  0,  h2,   0);

    // Right face (+X)
    glNormal3f(nXZ, nY,   0);
    glVertex3f( b2, -h2,  b2);
    glVertex3f( b2, -h2, -b2);
    glVertex3f(  0,  h2,   0);

    // Back face (-Z)
    glNormal3f(0,   nY, -nXZ);
    glVertex3f( b2, -h2, -b2);
    glVertex3f(-b2, -h2, -b2);
    glVertex3f(  0,  h2,   0);

    // Left face (-X)
    glNormal3f(-nXZ, nY,  0);
    glVertex3f(-b2, -h2, -b2);
    glVertex3f(-b2, -h2,  b2);
    glVertex3f(  0,  h2,   0);

    glEnd();

    // Square base (-Y face)
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-b2, -h2, -b2);
    glVertex3f( b2, -h2, -b2);
    glVertex3f( b2, -h2,  b2);
    glVertex3f(-b2, -h2,  b2);
    glEnd();
}

// ── Floor ─────────────────────────────────────────────────────────────────────
void drawFloor(float width, float depth, int divisions) {
    float hw   = width * 0.5f;
    float hd   = depth * 0.5f;
    float stepW = width / (float)divisions;
    float stepD = depth / (float)divisions;

    glNormal3f(0, 1, 0);   // all floor normals point up
    glBegin(GL_QUADS);
    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -hw + i * stepW;
            float z0 = -hd + j * stepD;
            glVertex3f(x0,        0, z0       );
            glVertex3f(x0+stepW,  0, z0       );
            glVertex3f(x0+stepW,  0, z0+stepD );
            glVertex3f(x0,        0, z0+stepD );
        }
    }
    glEnd();
}

// ── Torus ─────────────────────────────────────────────────────────────────────
/*
 * Torus parametrisation:
 *   x = (R + r*cos(phi)) * cos(theta)
 *   y = (R + r*cos(phi)) * sin(theta)  ← around XY, then we rotate
 *   z = r * sin(phi)
 * Normal at surface: (cos(phi)*cos(theta), cos(phi)*sin(theta), sin(phi))
 */
void drawTorus(float innerR, float outerR, int sides, int rings) {
    float pi2 = 2.f * (float)M_PI;

    for (int i = 0; i < rings; i++) {
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= sides; j++) {
            for (int k = 0; k <= 1; k++) {
                float theta = ((float)(i + k) / (float)rings) * pi2;
                float phi   = ((float)j       / (float)sides) * pi2;

                float cosT = std::cos(theta), sinT = std::sin(theta);
                float cosP = std::cos(phi),   sinP = std::sin(phi);

                float nx = cosP * cosT;
                float ny = cosP * sinT;
                float nz = sinP;
                glNormal3f(nx, ny, nz);

                float px = (outerR + innerR * cosP) * cosT;
                float py = (outerR + innerR * cosP) * sinT;
                float pz = innerR * sinP;
                glVertex3f(px, py, pz);
            }
        }
        glEnd();
    }
}

// ── Debug axes ────────────────────────────────────────────────────────────────
void drawAxes(float len) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.f);
    glBegin(GL_LINES);
        glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(len,0,0);   // X = red
        glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,len,0);   // Y = green
        glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,len);   // Z = blue
    glEnd();
    glEnable(GL_LIGHTING);
    glLineWidth(1.f);
}

} // namespace Primitives
