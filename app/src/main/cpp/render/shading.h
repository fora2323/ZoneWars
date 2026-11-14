#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "core/math.h"

constexpr int MAX_VERTICES = 32000;
constexpr int MAX_INDICES = 96000;

inline float v_buffer[MAX_VERTICES * 3];
inline float n_buffer[MAX_VERTICES * 3];
inline unsigned char c_buffer[MAX_VERTICES * 4];
inline unsigned short i_buffer[MAX_INDICES];
inline Mesh gCharMesh = {0};
inline Material gCharMat = {0};
inline int gVtxCount = 0;
inline int gIdxCount = 0;

struct Vtx { Vector3 p, n; Color c; };

inline void EmitVtxRaw(Vector3 p, Vector3 n, Color c) {
    if (gVtxCount >= MAX_VERTICES) return;
    int vIdx = gVtxCount * 3;
    v_buffer[vIdx] = p.x; v_buffer[vIdx+1] = p.y; v_buffer[vIdx+2] = p.z;
    n_buffer[vIdx] = n.x; n_buffer[vIdx+1] = n.y; n_buffer[vIdx+2] = n.z;
    int cIdx = gVtxCount * 4;
    c_buffer[cIdx] = c.r; c_buffer[cIdx+1] = c.g; c_buffer[cIdx+2] = c.b; c_buffer[cIdx+3] = c.a;
    gVtxCount++;
}

inline void EmitQuad(Vtx v1, Vtx v2, Vtx v3, Vtx v4) {
    if (gVtxCount + 4 > MAX_VERTICES || gIdxCount + 6 > MAX_INDICES) return;
    unsigned short start = (unsigned short)gVtxCount;
    EmitVtxRaw(v1.p, v1.n, v1.c); EmitVtxRaw(v2.p, v2.n, v2.c);
    EmitVtxRaw(v3.p, v3.n, v3.c); EmitVtxRaw(v4.p, v4.n, v4.c);
    i_buffer[gIdxCount++] = start;   i_buffer[gIdxCount++] = start+1; i_buffer[gIdxCount++] = start+2;
    i_buffer[gIdxCount++] = start;   i_buffer[gIdxCount++] = start+2; i_buffer[gIdxCount++] = start+3;
}

inline Color ColorLerp(Color a, Color b, float t) {
    return {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

inline void InitDynamicMesh() {
    gCharMesh = {0};
    gCharMesh.vertexCount = MAX_VERTICES;
    gCharMesh.triangleCount = MAX_INDICES / 3;
    gCharMesh.vertices = (float*)MemAlloc(MAX_VERTICES * 3 * sizeof(float));
    gCharMesh.normals = (float*)MemAlloc(MAX_VERTICES * 3 * sizeof(float));
    gCharMesh.colors = (unsigned char*)MemAlloc(MAX_VERTICES * 4);
    gCharMesh.indices = (unsigned short*)MemAlloc(MAX_INDICES * sizeof(unsigned short));
    UploadMesh(&gCharMesh, true);
    gCharMat = LoadMaterialDefault();
}

inline void BeginDynamicMesh() {
    gVtxCount = 0;
    gIdxCount = 0;
}

inline void EndDynamicMesh() {
    if (gVtxCount > 0) {
        UpdateMeshBuffer(gCharMesh, 0, v_buffer, gVtxCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(gCharMesh, 2, n_buffer, gVtxCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(gCharMesh, 3, c_buffer, gVtxCount * 4, 0);
        UpdateMeshBuffer(gCharMesh, 6, i_buffer, gIdxCount * sizeof(unsigned short), 0);
        int pv = gCharMesh.vertexCount;
        int pt = gCharMesh.triangleCount;
        gCharMesh.vertexCount = gVtxCount;
        gCharMesh.triangleCount = gIdxCount / 3;
        DrawMesh(gCharMesh, gCharMat, MatrixIdentity());
        gCharMesh.vertexCount = pv;
        gCharMesh.triangleCount = pt;
    }
}