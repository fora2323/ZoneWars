#pragma once
#include "render/shading.h"

inline void DrawDropShadow(Vector3 pos, float radius) {
    Color sc = {10, 10, 10, 120};
    Vector3 n = {0, 1, 0};
    float y = 0.01f;
    Vtx v1 = { {pos.x - radius, y, pos.z - radius}, n, sc };
    Vtx v2 = { {pos.x - radius, y, pos.z + radius}, n, sc };
    Vtx v3 = { {pos.x + radius, y, pos.z + radius}, n, sc };
    Vtx v4 = { {pos.x + radius, y, pos.z - radius}, n, sc };
    EmitQuad(v1, v2, v3, v4);
}