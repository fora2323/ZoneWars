#pragma once
#include "raylib.h"

inline void DrawUIButton(Rectangle r, const char* label, bool active, Color activeColor) {
    Color bg = active ? activeColor : Color{100, 100, 100, 150};
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 2.0f, WHITE);
    int tw = MeasureText(label, 20);
    DrawText(label, (int)(r.x + r.width/2 - tw/2), (int)(r.y + r.height/2 - 10), 20, WHITE);
}

inline bool IsPointInAnyUI(Vector2 pt, const Rectangle* rects, int count) {
    for(int i = 0; i < count; ++i) {
        if (CheckCollisionPointRec(pt, rects[i])) return true;
    }
    return false;
}