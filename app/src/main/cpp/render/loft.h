#pragma once
#include "render/shading.h"

struct LoftSlice { Vector3 center; float radius; Color col; };

constexpr int MAX_LOFT_SLICES = 128;
constexpr float kTwoPi = 6.2831853f;

struct SmoothResult { LoftSlice slices[MAX_LOFT_SLICES]; int count; };

inline float CatmullRomF(float p0, float p1, float p2, float p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1)
                 + (-p0 + p2) * t
                 + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2
                 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

inline Vector3 CatmullRomV(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t) {
    return Vector3{
        CatmullRomF(p0.x, p1.x, p2.x, p3.x, t),
        CatmullRomF(p0.y, p1.y, p2.y, p3.y, t),
        CatmullRomF(p0.z, p1.z, p2.z, p3.z, t)
    };
}

inline SmoothResult SmoothSlices(const LoftSlice* input, int count, int subdiv) {
    SmoothResult res;
    res.count = 0;
    if (count < 2 || subdiv < 1) return res;
    for (int i = 0; i < count - 1; ++i) {
        const LoftSlice& p0 = input[i == 0 ? 0 : i - 1];
        const LoftSlice& p1 = input[i];
        const LoftSlice& p2 = input[i + 1];
        const LoftSlice& p3 = input[i + 2 >= count ? count - 1 : i + 2];
        int steps = (i == count - 2) ? subdiv + 1 : subdiv;
        for (int s = 0; s < steps; ++s) {
            if (res.count >= MAX_LOFT_SLICES) return res;
            float t = (float)s / (float)subdiv;
            LoftSlice out;
            out.center = CatmullRomV(p0.center, p1.center, p2.center, p3.center, t);
            out.radius = fmaxf(0.0f, CatmullRomF(p0.radius, p1.radius, p2.radius, p3.radius, t));
            out.col = ColorLerp(p1.col, p2.col, t);
            res.slices[res.count++] = out;
        }
    }
    return res;
}

inline void DrawLoft(const LoftSlice* slices, int count, Basis3D basis) {
    if (count < 2) return;
    constexpr int SEGS = 16;
    for (int i = 0; i < count - 1; ++i) {
        Vector3 dV = VSub(slices[i + 1].center, slices[i].center);
        float dR = slices[i + 1].radius - slices[i].radius;
        Vector3 dVPrev = i > 0 ? VSub(slices[i].center, slices[i - 1].center) : dV;
        Vector3 dVNext = i < count - 2 ? VSub(slices[i + 2].center, slices[i + 1].center) : dV;
        Vector3 t0 = VNorm(VAdd(dV, dVPrev));
        Vector3 t1 = VNorm(VAdd(dV, dVNext));
        for (int j = 0; j < SEGS; ++j) {
            float a0 = kTwoPi * (float)j / (float)SEGS;
            float a1 = kTwoPi * (float)(j + 1) / (float)SEGS;
            Vector3 u0 = VAdd(VMul(basis.right, cosf(a0)), VMul(basis.up, sinf(a0)));
            Vector3 u1 = VAdd(VMul(basis.right, cosf(a1)), VMul(basis.up, sinf(a1)));
            Vector3 n00 = VNorm(VCross(VAdd(t0, VMul(u0, dR)), VNorm(VCross(t0, u0))));
            Vector3 n01 = VNorm(VCross(VAdd(t0, VMul(u1, dR)), VNorm(VCross(t0, u1))));
            Vector3 n10 = VNorm(VCross(VAdd(t1, VMul(u0, dR)), VNorm(VCross(t1, u0))));
            Vector3 n11 = VNorm(VCross(VAdd(t1, VMul(u1, dR)), VNorm(VCross(t1, u1))));
            if (VDot(n00, u0) < 0.0f) n00 = VMul(n00, -1.0f);
            if (VDot(n01, u1) < 0.0f) n01 = VMul(n01, -1.0f);
            if (VDot(n10, u0) < 0.0f) n10 = VMul(n10, -1.0f);
            if (VDot(n11, u1) < 0.0f) n11 = VMul(n11, -1.0f);
            Vtx v00 = { VAdd(slices[i].center, VMul(u0, slices[i].radius)), n00, slices[i].col };
            Vtx v01 = { VAdd(slices[i].center, VMul(u1, slices[i].radius)), n01, slices[i].col };
            Vtx v10 = { VAdd(slices[i + 1].center, VMul(u0, slices[i + 1].radius)), n10, slices[i + 1].col };
            Vtx v11 = { VAdd(slices[i + 1].center, VMul(u1, slices[i + 1].radius)), n11, slices[i + 1].col };
            EmitQuad(v00, v01, v11, v10);
        }
    }
}

inline void DrawSmoothLoft(const LoftSlice* input, int count, int subdiv, Basis3D basis) {
    SmoothResult res = SmoothSlices(input, count, subdiv);
    DrawLoft(res.slices, res.count, basis);
}