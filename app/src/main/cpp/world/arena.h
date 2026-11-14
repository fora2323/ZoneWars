#pragma once
#include "raymath.h"
#include "render/shading.h"

extern Mesh gArenaMesh;
extern Material gArenaMat;

inline void InitArenaMesh() {
    gArenaMesh = {0};
    gArenaMesh.vertexCount = 32 * 32 * 6 + 4 * 6;
    gArenaMesh.triangleCount = gArenaMesh.vertexCount / 3;
    gArenaMesh.vertices = (float*)MemAlloc(gArenaMesh.vertexCount * 3 * sizeof(float));
    gArenaMesh.normals = (float*)MemAlloc(gArenaMesh.vertexCount * 3 * sizeof(float));
    gArenaMesh.colors = (unsigned char*)MemAlloc(gArenaMesh.vertexCount * 4);
    gArenaMesh.indices = (unsigned short*)MemAlloc(gArenaMesh.vertexCount * sizeof(unsigned short));
    
    int vc = 0;
    auto addQ = [&](Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, Color c, Vector3 n) {
        float* v = gArenaMesh.vertices; float* nm = gArenaMesh.normals; unsigned char* cl = gArenaMesh.colors;
        unsigned short* id = gArenaMesh.indices;
        int i0 = vc;
        v[vc*3]=p1.x; v[vc*3+1]=p1.y; v[vc*3+2]=p1.z; nm[vc*3]=n.x; nm[vc*3+1]=n.y; nm[vc*3+2]=n.z; cl[vc*4]=c.r; cl[vc*4+1]=c.g; cl[vc*4+2]=c.b; cl[vc*4+3]=c.a; vc++;
        v[vc*3]=p2.x; v[vc*3+1]=p2.y; v[vc*3+2]=p2.z; nm[vc*3]=n.x; nm[vc*3+1]=n.y; nm[vc*3+2]=n.z; cl[vc*4]=c.r; cl[vc*4+1]=c.g; cl[vc*4+2]=c.b; cl[vc*4+3]=c.a; vc++;
        v[vc*3]=p3.x; v[vc*3+1]=p3.y; v[vc*3+2]=p3.z; nm[vc*3]=n.x; nm[vc*3+1]=n.y; nm[vc*3+2]=n.z; cl[vc*4]=c.r; cl[vc*4+1]=c.g; cl[vc*4+2]=c.b; cl[vc*4+3]=c.a; vc++;
        v[vc*3]=p4.x; v[vc*3+1]=p4.y; v[vc*3+2]=p4.z; nm[vc*3]=n.x; nm[vc*3+1]=n.y; nm[vc*3+2]=n.z; cl[vc*4]=c.r; cl[vc*4+1]=c.g; cl[vc*4+2]=c.b; cl[vc*4+3]=c.a; vc++;
        id[i0]=i0; id[i0+1]=i0+1; id[i0+2]=i0+2; id[i0+3]=i0; id[i0+4]=i0+2; id[i0+5]=i0+3;
    };
    
    Color c1 = {46, 52, 62, 255};
    Color c2 = {55, 62, 74, 255};
    for (int x = -16; x < 16; ++x) {
        for (int z = -16; z < 16; ++z) {
            Color cc = ((x + z) % 2 == 0) ? c1 : c2;
            float fx = x * 4.0f, fz = z * 4.0f;
            addQ({fx, 0, fz}, {fx, 0, fz+4.0f}, {fx+4.0f, 0, fz+4.0f}, {fx+4.0f, 0, fz}, cc, {0, 1, 0});
        }
    }
    
    Color cw = {185, 45, 58, 255};
    float w = 60.0f, h = 0.24f, t = 1.0f;
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