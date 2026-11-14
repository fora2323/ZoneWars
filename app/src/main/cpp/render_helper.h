#pragma once

#include "raylib.h"
#include "rlgl.h"
#include "math_anim_helper.h"

inline Color ShadeStudio(Color base, Vector3 normal, float specStrength = 0.15f)
{
    const Vector3 lightDir = V3(0.38f, 0.62f, 0.68f);
    const Vector3 halfVec  = V3(0.20f, 0.45f, 0.87f);

    float ndl = VDot(normal, lightDir);
    float diff = Clamp01(ndl * 0.50f + 0.50f);

    float ndh = Clamp01(VDot(normal, halfVec));
    float s2 = ndh * ndh;
    float s4 = s2 * s2;
    float s8 = s4 * s4;
    float spec = (s8 * s8) * specStrength;

    float shade = 0.72f + 0.32f * diff;
    int r = (int)Clampf(base.r * shade + spec * 230.0f, 0.0f, 255.0f);
    int g = (int)Clampf(base.g * shade + spec * 220.0f, 0.0f, 255.0f);
    int b = (int)Clampf(base.b * shade + spec * 225.0f, 0.0f, 255.0f);
    return Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, base.a };
}

struct LoftSlice {
    Vector3 center;
    float rx;
    float rz;
    Color color;
    float spec;
};

inline Vector3 LoftNormal(const LoftSlice& s0, const LoftSlice& s1, float a)
{
    float ca = cosf(a);
    float sa = sinf(a);
    float rx = 0.5f * (s0.rx + s1.rx);
    float rz = 0.5f * (s0.rz + s1.rz);
    Vector3 tAngle = V3(-sa * rx, 0.0f, ca * rz);
    Vector3 p0 = V3(s0.center.x + ca * s0.rx, s0.center.y, s0.center.z + sa * s0.rz);
    Vector3 p1 = V3(s1.center.x + ca * s1.rx, s1.center.y, s1.center.z + sa * s1.rz);
    Vector3 tVert = VSub(p1, p0);
    Vector3 n = VCross(tAngle, tVert);
    n = VMul(n, -1.0f);
    float l = VLen(n);
    return l > 1e-6f ? VMul(n, 1.0f / l) : V3(ca, 0.0f, sa);
}

inline Vector3 EllipsoidNormal(Vector3 ln, Vector3 radii)
{
    float rx = radii.x > 1e-6f ? radii.x : 1e-6f;
    float ry = radii.y > 1e-6f ? radii.y : 1e-6f;
    float rz = radii.z > 1e-6f ? radii.z : 1e-6f;
    return VNorm(V3(ln.x / rx, ln.y / ry, ln.z / rz));
}

inline void DrawLoftedMesh(Vector3 rootPos, float rootYaw, const LoftSlice* slices, int sliceCount, int segs = 12)
{
    if (sliceCount < 2) return;
    const float twoPi = 6.2831853f;

    rlDisableBackfaceCulling();
    rlBegin(RL_TRIANGLES);
    for (int s = 0; s < sliceCount - 1; s++) {
        const LoftSlice& s0 = slices[s];
        const LoftSlice& s1 = slices[s + 1];

        for (int i = 0; i < segs; i++) {
            float a0 = (float)i / (float)segs * twoPi;
            float a1 = (float)(i + 1) / (float)segs * twoPi;

            float c0 = cosf(a0), sn0 = sinf(a0);
            float c1 = cosf(a1), sn1 = sinf(a1);

            Vector3 p00 = WorldPoint(rootPos, V3(s0.center.x + c0 * s0.rx, s0.center.y, s0.center.z + sn0 * s0.rz), rootYaw);
            Vector3 p01 = WorldPoint(rootPos, V3(s0.center.x + c1 * s0.rx, s0.center.y, s0.center.z + sn1 * s0.rz), rootYaw);
            Vector3 p10 = WorldPoint(rootPos, V3(s1.center.x + c0 * s1.rx, s1.center.y, s1.center.z + sn0 * s1.rz), rootYaw);
            Vector3 p11 = WorldPoint(rootPos, V3(s1.center.x + c1 * s1.rx, s1.center.y, s1.center.z + sn1 * s1.rz), rootYaw);

            Vector3 n0 = RotateYaw(LoftNormal(s0, s1, a0), rootYaw);
            Vector3 n1 = RotateYaw(LoftNormal(s0, s1, a1), rootYaw);

            Color col00 = ShadeStudio(s0.color, n0, s0.spec);
            Color col01 = ShadeStudio(s0.color, n1, s0.spec);
            Color col10 = ShadeStudio(s1.color, n0, s1.spec);
            Color col11 = ShadeStudio(s1.color, n1, s1.spec);

            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col01.r, col01.g, col01.b, col01.a); rlVertex3f(p01.x, p01.y, p01.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);

            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);
            rlColor4ub(col10.r, col10.g, col10.b, col10.a); rlVertex3f(p10.x, p10.y, p10.z);
        }
    }
    rlEnd();
}

inline void DrawBasisLoft(const Basis3D& B, const LoftSlice* slices, int sliceCount, int segs = 14)
{
    if (sliceCount < 2) return;
    const float twoPi = 6.2831853f;

    rlDisableBackfaceCulling();
    rlBegin(RL_TRIANGLES);
    for (int s = 0; s < sliceCount - 1; s++) {
        const LoftSlice& s0 = slices[s];
        const LoftSlice& s1 = slices[s + 1];

        for (int i = 0; i < segs; i++) {
            float a0 = (float)i / (float)segs * twoPi;
            float a1 = (float)(i + 1) / (float)segs * twoPi;

            float c0 = cosf(a0), sn0 = sinf(a0);
            float c1 = cosf(a1), sn1 = sinf(a1);

            Vector3 p00 = BasisPoint(B, s0.center.x + c0 * s0.rx, s0.center.y, s0.center.z + sn0 * s0.rz);
            Vector3 p01 = BasisPoint(B, s0.center.x + c1 * s0.rx, s0.center.y, s0.center.z + sn1 * s0.rz);
            Vector3 p10 = BasisPoint(B, s1.center.x + c0 * s1.rx, s1.center.y, s1.center.z + sn0 * s1.rz);
            Vector3 p11 = BasisPoint(B, s1.center.x + c1 * s1.rx, s1.center.y, s1.center.z + sn1 * s1.rz);

            Vector3 ln0 = LoftNormal(s0, s1, a0);
            Vector3 ln1 = LoftNormal(s0, s1, a1);
            Vector3 n0 = BasisDir(B, ln0.x, ln0.y, ln0.z);
            Vector3 n1 = BasisDir(B, ln1.x, ln1.y, ln1.z);

            Color col00 = ShadeStudio(s0.color, n0, s0.spec);
            Color col01 = ShadeStudio(s0.color, n1, s0.spec);
            Color col10 = ShadeStudio(s1.color, n0, s1.spec);
            Color col11 = ShadeStudio(s1.color, n1, s1.spec);

            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col01.r, col01.g, col01.b, col01.a); rlVertex3f(p01.x, p01.y, p01.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);

            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);
            rlColor4ub(col10.r, col10.g, col10.b, col10.a); rlVertex3f(p10.x, p10.y, p10.z);
        }
    }
    rlEnd();
}

inline void DrawLitLimb(Vector3 rootPos, float rootYaw, Vector3 a, Vector3 b,
                        float rA, float rB, Color colA, Color colB, float spec = 0.2f, int segs = 10)
{
    Vector3 wa = WorldPoint(rootPos, a, rootYaw);
    Vector3 wb = WorldPoint(rootPos, b, rootYaw);
    Vector3 axis = VSub(wb, wa);
    float len = VLen(axis);
    if (len < 1e-5f) return;
    Vector3 up = VMul(axis, 1.0f / len);

    Vector3 ref = fabsf(up.y) < 0.9f ? V3(0.0f, 1.0f, 0.0f) : V3(1.0f, 0.0f, 0.0f);
    Vector3 right = VNorm(VCross(up, ref));
    Vector3 fwd   = VNorm(VCross(right, up));

    const float twoPi = 6.2831853f;
    rlDisableBackfaceCulling();
    rlBegin(RL_TRIANGLES);
    for (int i = 0; i < segs; i++) {
        float a0 = (float)i / (float)segs * twoPi;
        float a1 = (float)(i + 1) / (float)segs * twoPi;

        Vector3 n0 = VAdd(VMul(right, cosf(a0)), VMul(fwd, sinf(a0)));
        Vector3 n1 = VAdd(VMul(right, cosf(a1)), VMul(fwd, sinf(a1)));

        Vector3 p00 = VAdd(wa, VMul(n0, rA));
        Vector3 p01 = VAdd(wa, VMul(n1, rA));
        Vector3 p10 = VAdd(wb, VMul(n0, rB));
        Vector3 p11 = VAdd(wb, VMul(n1, rB));

        Color c00 = ShadeStudio(colA, n0, spec);
        Color c01 = ShadeStudio(colA, n1, spec);
        Color c10 = ShadeStudio(colB, n0, spec);
        Color c11 = ShadeStudio(colB, n1, spec);

        rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
        rlColor4ub(c01.r, c01.g, c01.b, c01.a); rlVertex3f(p01.x, p01.y, p01.z);
        rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);

        rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
        rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);
        rlColor4ub(c10.r, c10.g, c10.b, c10.a); rlVertex3f(p10.x, p10.y, p10.z);
    }
    rlEnd();
}

inline void DrawLitEllipsoid(const Basis3D& B, Vector3 localCenter, Vector3 radii, Color baseCol, float spec = 0.2f, int rings = 7, int slices = 10)
{
    const float pi = 3.14159265f;
    const float twoPi = 6.2831853f;

    rlDisableBackfaceCulling();
    rlBegin(RL_TRIANGLES);
    for (int r = 0; r < rings; r++) {
        float v0 = (float)r / (float)rings * pi - pi * 0.5f;
        float v1 = (float)(r + 1) / (float)rings * pi - pi * 0.5f;
        float cv0 = cosf(v0), sv0 = sinf(v0);
        float cv1 = cosf(v1), sv1 = sinf(v1);

        for (int s = 0; s < slices; s++) {
            float u0 = (float)s / (float)slices * twoPi;
            float u1 = (float)(s + 1) / (float)slices * twoPi;
            float cu0 = cosf(u0), su0 = sinf(u0);
            float cu1 = cosf(u1), su1 = sinf(u1);

            Vector3 ln00 = V3(cv0 * cu0, sv0, cv0 * su0);
            Vector3 ln01 = V3(cv0 * cu1, sv0, cv0 * su1);
            Vector3 ln10 = V3(cv1 * cu0, sv1, cv1 * su0);
            Vector3 ln11 = V3(cv1 * cu1, sv1, cv1 * su1);

            Vector3 p00 = BasisPoint(B, localCenter.x + ln00.x * radii.x, localCenter.y + ln00.y * radii.y, localCenter.z + ln00.z * radii.z);
            Vector3 p01 = BasisPoint(B, localCenter.x + ln01.x * radii.x, localCenter.y + ln01.y * radii.y, localCenter.z + ln01.z * radii.z);
            Vector3 p10 = BasisPoint(B, localCenter.x + ln10.x * radii.x, localCenter.y + ln10.y * radii.y, localCenter.z + ln10.z * radii.z);
            Vector3 p11 = BasisPoint(B, localCenter.x + ln11.x * radii.x, localCenter.y + ln11.y * radii.y, localCenter.z + ln11.z * radii.z);

            Vector3 en00 = EllipsoidNormal(ln00, radii);
            Vector3 en01 = EllipsoidNormal(ln01, radii);
            Vector3 en10 = EllipsoidNormal(ln10, radii);
            Vector3 en11 = EllipsoidNormal(ln11, radii);

            Color c00 = ShadeStudio(baseCol, BasisDir(B, en00.x, en00.y, en00.z), spec);
            Color c01 = ShadeStudio(baseCol, BasisDir(B, en01.x, en01.y, en01.z), spec);
            Color c10 = ShadeStudio(baseCol, BasisDir(B, en10.x, en10.y, en10.z), spec);
            Color c11 = ShadeStudio(baseCol, BasisDir(B, en11.x, en11.y, en11.z), spec);

            rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(c01.r, c01.g, c01.b, c01.a); rlVertex3f(p01.x, p01.y, p01.z);
            rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);

            rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);
            rlColor4ub(c10.r, c10.g, c10.b, c10.a); rlVertex3f(p10.x, p10.y, p10.z);
        }
    }
    rlEnd();
}

inline void DrawBasisTube(const Basis3D& B, Vector3 a, Vector3 b, float r, Color col, float spec = 0.4f)
{
    Vector3 wa = BasisPoint(B, a.x, a.y, a.z);
    Vector3 wb = BasisPoint(B, b.x, b.y, b.z);
    Color lit = ShadeStudio(col, B.forward, spec);
    DrawCylinderEx(wa, wb, r, r, 12, lit);
}

inline Vector3 ProjectFloorShadow(Vector3 worldPt)
{
    float h = fmaxf(0.0f, worldPt.y);
    return V3(worldPt.x + h * 0.36f, 0.015f, worldPt.z + h * 0.26f);
}

inline void DrawFlatShadowRibbon(Vector3 pos, float yaw, Vector3 a, Vector3 b, float width, Color sc)
{
    Vector3 p0 = ProjectFloorShadow(WorldPoint(pos, a, yaw));
    Vector3 p1 = ProjectFloorShadow(WorldPoint(pos, b, yaw));
    Vector3 dir = VSub(p1, p0);
    dir.y = 0.0f;
    float len = VLen(dir);
    if (len < 0.001f) return;
    Vector3 perp = V3(-dir.z / len * width, 0.0f, dir.x / len * width);

    Vector3 v0 = VSub(p0, perp);
    Vector3 v1 = VAdd(p0, perp);
    Vector3 v2 = VAdd(p1, perp);
    Vector3 v3 = VSub(p1, perp);

    rlColor4ub(sc.r, sc.g, sc.b, sc.a);
    rlVertex3f(v0.x, v0.y, v0.z);
    rlVertex3f(v1.x, v1.y, v1.z);
    rlVertex3f(v2.x, v2.y, v2.z);

    rlVertex3f(v0.x, v0.y, v0.z);
    rlVertex3f(v2.x, v2.y, v2.z);
    rlVertex3f(v3.x, v3.y, v3.z);
}

inline void DrawFemaleShadow(const FemalePose& P, Vector3 pos, float yaw)
{
    Color shadowCol = Color{ 28, 32, 40, 255 };

    rlDisableBackfaceCulling();
    rlBegin(RL_TRIANGLES);
    DrawFlatShadowRibbon(pos, yaw, P.pelvis, P.headCenter, 0.14f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.lShoulder, P.lElbow, 0.05f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.lElbow, P.lHand, 0.04f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.rShoulder, P.rElbow, 0.05f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.rElbow, P.rHand, 0.04f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.lHip, P.lKnee, 0.075f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.lKnee, P.lAnkle, 0.055f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.rHip, P.rKnee, 0.075f, shadowCol);
    DrawFlatShadowRibbon(pos, yaw, P.rKnee, P.rAnkle, 0.055f, shadowCol);
    rlEnd();
}