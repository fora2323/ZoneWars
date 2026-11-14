#pragma once
#include "math.h"

struct Player {
    Vector3 pos;
    Vector3 vel;
    float yaw;
    float pitch;
    int state; 
    bool isSpinning;
    float spinAngle;
    double lastJumpTime;
    int jumpCount;
};

inline void DrawSausageMan(const Player& p) {
    float h = 1.0f;
    float r = 0.35f;
    Vector3 center = VAdd(p.pos, {0, h / 2.0f + r, 0});
    Color skin = {240, 150, 50, 255};
    Color eye = {30, 30, 30, 255};

    if (p.state == 3) {
        center.y = r + 0.1f;
        Vector3 dir = RotateYaw({0, 0, 1}, p.yaw);
        Vector3 p1 = VAdd(center, VMul(dir, h / 2.0f));
        Vector3 p2 = VSub(center, VMul(dir, h / 2.0f));
        DrawCapsule(p2, p1, r, 6, 6, skin);
    } else {
        if (p.state == 2) h = 0.4f;
        
        Vector3 localUp = {0, cosf(p.spinAngle), sinf(p.spinAngle)};
        Vector3 worldUp = RotateYaw(localUp, p.yaw);
        
        Vector3 p1 = VAdd(center, VMul(worldUp, h / 2.0f));
        Vector3 p2 = VSub(center, VMul(worldUp, h / 2.0f));
        
        DrawCapsule(p2, p1, r, 6, 6, skin);
        
        Vector3 faceDir = RotateYaw({0, -sinf(p.spinAngle), cosf(p.spinAngle)}, p.yaw);
        Vector3 right = RotateYaw({1, 0, 0}, p.yaw);
        
        Vector3 e1 = VAdd(VAdd(p1, VMul(faceDir, r * 0.85f)), VMul(right, 0.15f));
        Vector3 e2 = VAdd(VAdd(p1, VMul(faceDir, r * 0.85f)), VMul(right, -0.15f));
        DrawCube(e1, 0.12f, 0.12f, 0.12f, eye);
        DrawCube(e2, 0.12f, 0.12f, 0.12f, eye);
    }
}

inline void DrawPolygonWorld() {
    DrawGrid(40, 4.0f);
    DrawCube({0, -0.5f, 0}, 160.0f, 1.0f, 160.0f, {80, 160, 90, 255});
    DrawCube({10, 2.0f, 10}, 4.0f, 4.0f, 4.0f, {180, 80, 80, 255});
    DrawCube({-15, 1.0f, 20}, 8.0f, 2.0f, 6.0f, {80, 100, 180, 255});
    DrawCube({20, 3.0f, -15}, 6.0f, 6.0f, 6.0f, {180, 150, 80, 255});
}

inline void DrawUIButton(Rectangle r, const char* text, bool active) {
    DrawRectangleRec(r, active ? Color{200, 50, 50, 200} : Color{100, 100, 100, 150});
    DrawRectangleLinesEx(r, 3.0f, WHITE);
    int tw = MeasureText(text, 24);
    DrawText(text, (int)(r.x + r.width / 2 - tw / 2), (int)(r.y + r.height / 2 - 12), 24, WHITE);
}