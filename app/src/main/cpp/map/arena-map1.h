#pragma once
#include "raylib.h"

inline void DrawPolygonWorld() {
    // Permukaan tanah dinaikkan sedikit agar tidak tumpang tindih dengan grid (menghilangkan bling/z-fighting)
    DrawGrid(40, 4.0f);
    DrawCube({0, -0.51f, 0}, 160.0f, 1.0f, 160.0f, {80, 160, 90, 255});
    DrawCube({10, 2.0f, 10}, 4.0f, 4.0f, 4.0f, {180, 80, 80, 255});
    DrawCube({-15, 1.0f, 20}, 8.0f, 2.0f, 6.0f, {80, 100, 180, 255});
    DrawCube({20, 3.0f, -15}, 6.0f, 6.0f, 6.0f, {180, 150, 80, 255});
}