#pragma once
#include "render/loft.h"

inline void DrawBasisTube(Vector3 a, Vector3 b, float rA, float rB, Color cA, Color cB) {
    Vector3 dir = VSub(b, a);
    float len = VLen(dir);
    if (len < 0.001f) return;
    dir = VMul(dir, 1.0f / len);
    LoftSlice slices[2] = { {a, rA, cA}, {b, rB, cB} };
    DrawLoft(slices, 2, MakeBasis(dir));
}

inline void DrawLitLimb(Vector3 a, Vector3 b, float rA, float rB, Color cA, Color cB) {
    Vector3 dir = VSub(b, a);
    float len = VLen(dir);
    if (len < 0.001f) return;
    dir = VMul(dir, 1.0f / len);
    LoftSlice slices[10];
    int count = 0;
    for(int i = 3; i >= 1; --i) {
        float lat = (M_PI / 2.0f) * (i / 3.0f);
        slices[count++] = { VAdd(a, VMul(dir, -sinf(lat)*rA)), cosf(lat)*rA, cA };
    }
    slices[count++] = { a, rA, cA };
    slices[count++] = { b, rB, cB };
    for(int i = 1; i <= 3; ++i) {
        float lat = (M_PI / 2.0f) * (i / 3.0f);
        slices[count++] = { VAdd(b, VMul(dir, sinf(lat)*rB)), cosf(lat)*rB, cB };
    }
    DrawLoft(slices, count, MakeBasis(dir));
}

inline void DrawLitEllipsoid(Vector3 center, Vector3 rad, Basis3D b, Color col) {
    constexpr int LAT = 8, LON = 12;
    for (int i = 0; i < LAT; ++i) {
        float lat0 = -M_PI/2.0f + M_PI * (float)i / LAT;
        float lat1 = -M_PI/2.0f + M_PI * (float)(i+1) / LAT;
        for (int j = 0; j < LON; ++j) {
            float lon0 = M_PI * 2.0f * (float)j / LON;
            float lon1 = M_PI * 2.0f * (float)(j+1) / LON;
            Vector3 d00 = { cosf(lat0)*cosf(lon0), sinf(lat0), cosf(lat0)*sinf(lon0) };
            Vector3 d01 = { cosf(lat0)*cosf(lon1), sinf(lat0), cosf(lat0)*sinf(lon1) };
            Vector3 d10 = { cosf(lat1)*cosf(lon0), sinf(lat1), cosf(lat1)*sinf(lon0) };
            Vector3 d11 = { cosf(lat1)*cosf(lon1), sinf(lat1), cosf(lat1)*sinf(lon1) };
            Vector3 p00 = VAdd(center, BasisPoint(b, {d00.x*rad.x, d00.y*rad.y, d00.z*rad.z}));
            Vector3 p01 = VAdd(center, BasisPoint(b, {d01.x*rad.x, d01.y*rad.y, d01.z*rad.z}));
            Vector3 p10 = VAdd(center, BasisPoint(b, {d10.x*rad.x, d10.y*rad.y, d10.z*rad.z}));
            Vector3 p11 = VAdd(center, BasisPoint(b, {d11.x*rad.x, d11.y*rad.y, d11.z*rad.z}));
            Vector3 n00 = BasisDir(b, VNorm({d00.x/rad.x, d00.y/rad.y, d00.z/rad.z}));
            Vector3 n01 = BasisDir(b, VNorm({d01.x/rad.x, d01.y/rad.y, d01.z/rad.z}));
            Vector3 n10 = BasisDir(b, VNorm({d10.x/rad.x, d10.y/rad.y, d10.z/rad.z}));
            Vector3 n11 = BasisDir(b, VNorm({d11.x/rad.x, d11.y/rad.y, d11.z/rad.z}));
            EmitQuad({p00, n00, col}, {p01, n01, col}, {p11, n11, col}, {p10, n10, col});
        }
    }
}