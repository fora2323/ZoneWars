#pragma once
#include "raymath.h"
#include "render/shading.h"

extern Mesh gArenaMesh;
extern Material gArenaMat;

inline void InitArenaMesh() {
    const int quadCount = 32 * 32 + 4;
    gArenaMesh = {0};
    gArenaMesh.vertexCount = quadCount * 4;
    gArenaMesh.triangleCount = quadCount * 2;
    gArenaMesh.vertices = (float*)MemAlloc(gArenaMesh.vertexCount * 3 * sizeof(float));
    gArenaMesh.normals = (float*)MemAlloc(gArenaMesh.vertexCount * 3 * sizeof(float));
    gArenaMesh.colors = (unsigned char*)MemAlloc(gArenaMesh.vertexCount * 4);
    gArenaMesh.indices = (unsigned short*)MemAlloc(quadCount * 6 * sizeof(unsigned short));

    int vc = 0;
    int ic = 0;
    auto addQ = [&](Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, Color c, Vector3 n) {
        Vector3 ps[4] = { p1, p2, p3, p4 };
        for (int k = 0; k < 4; ++k) {
            int o = vc * 3;
            gArenaMesh.vertices[o] = ps[k].x;
            gArenaMesh.vertices[o + 1] = ps[k].y;
            gArenaMesh.vertices[o + 2] = ps[k].z;
            gArenaMesh.normals[o] = n.x;
            gArenaMesh.normals[o + 1] = n.y;
            gArenaMesh.normals[o + 2] = n.z;
            int co = vc * 4;
            gArenaMesh.colors[co] = c.r;
            gArenaMesh.colors[co + 1] = c.g;
            gArenaMesh.colors[co + 2] = c.b;
            gArenaMesh.colors[co + 3] = c.a;
            vc++;
        }
        unsigned short b = (unsigned short)(vc - 4);
        gArenaMesh.indices[ic++] = b;
        gArenaMesh.indices[ic++] = (unsigned short)(b + 1);
        gArenaMesh.indices[ic++] = (unsigned short)(b + 2);
        gArenaMesh.indices[ic++] = b;
        gArenaMesh.indices[ic++] = (unsigned short)(b + 2);
        gArenaMesh.indices[ic++] = (unsigned short)(b + 3);
    };

    Color c1 = {46, 52, 62, 255};
    Color c2 = {55, 62, 74, 255};
    for (int x = -16; x < 16; ++x) {
        for (int z = -16; z < 16; ++z) {
            Color cc = ((x + z) & 1) == 0 ? c1 : c2;
            float fx = x * 4.0f, fz = z * 4.0f;
            addQ({fx, 0, fz}, {fx, 0, fz + 4.0f}, {fx + 4.0f, 0, fz + 4.0f}, {fx + 4.0f, 0, fz}, cc, {0, 1, 0});
        }
    }

    Color cw = {185, 45, 58, 255};
    float w = 60.0f, h = 0.24f;
    addQ({-w, 0, -w}, {-w, h, -w}, {w, h, -w}, {w, 0, -w}, cw, {0, 0, 1});
    addQ({-w, 0, w}, {w, 0, w}, {w, h, w}, {-w, h, w}, cw, {0, 0, -1});
    addQ({-w, 0, -w}, {-w, 0, w}, {-w, h, w}, {-w, h, -w}, cw, {1, 0, 0});
    addQ({w, 0, -w}, {w, h, -w}, {w, h, w}, {w, 0, w}, cw, {-1, 0, 0});

    UploadMesh(&gArenaMesh, false);
    gArenaMat = LoadMaterialDefault();
}

inline void DrawFlatArenaMap(Vector3 playerPos) {
    DrawMesh(gArenaMesh, gArenaMat, MatrixIdentity());
}