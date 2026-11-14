#pragma once

#include "character.h"

inline Body FemaleBody() { return Body{ 0.19f, 0.15f, 0.15f }; }

inline Style FemaleStyle()
{
    Style s;
    s.skin = Color{ 240, 218, 200, 255 };
    s.top = Color{ 150, 22, 34, 255 };
    s.bottom = Color{ 30, 26, 30, 255 };
    s.skirtC = Color{ 150, 22, 34, 255 };
    s.hair = Color{ 18, 16, 20, 255 };
    s.shoe = Color{ 22, 20, 22, 255 };
    s.lip = Color{ 170, 40, 55, 255 };
    s.hand = Color{ 25, 25, 28, 255 };
    s.hairLen = 0.14f;
    s.skirt = true;
    s.boots = true;
    return s;
}

inline void DrawGlasses(const Pose& P, Vector3 pos, float yaw, float r)
{
    Color gold = Color{ 212, 175, 55, 255 };
    for (int k = -1; k <= 1; k += 2) {
        float sx = (float)k;
        float cx = 0.058f * sx;
        float cy = 0.012f;
        float hw = 0.042f;
        float hh = 0.022f;
        Vector3 tl = OnFace(P.head, r, cx - hw, cy + hh);
        Vector3 tr = OnFace(P.head, r, cx + hw, cy + hh);
        Vector3 br = OnFace(P.head, r, cx + hw, cy - hh);
        Vector3 bl = OnFace(P.head, r, cx - hw, cy - hh);
        Cap(pos, yaw, tl, tr, 0.0045f, gold);
        Cap(pos, yaw, tr, br, 0.0045f, gold);
        Cap(pos, yaw, br, bl, 0.0045f, gold);
        Cap(pos, yaw, bl, tl, 0.0045f, gold);

        Vector3 outer = sx < 0.0f ? tl : tr;
        Vector3 tip = V3(P.head.x + sx * (r + 0.02f), P.head.y + cy + 0.01f, P.head.z + 0.02f);
        Cap(pos, yaw, outer, tip, 0.0035f, gold);

        for (int i = 0; i < 6; i++) {
            float t = (float)i / 5.0f;
            Sph(pos, yaw, V3(P.head.x + sx * (r + 0.03f), P.head.y - 0.02f - t * 0.3f, P.head.z - 0.01f - t * 0.04f), 0.006f, gold);
        }
    }
    Cap(pos, yaw, OnFace(P.head, r, -0.014f, 0.022f), OnFace(P.head, r, 0.014f, 0.022f), 0.0035f, gold);
}

inline void DrawFemaleCharacter(const Pose& P, Vector3 pos, float yaw, float blink)
{
    Body b = FemaleBody();
    DrawHumanoid(P, pos, yaw, FemaleStyle(), b, blink);
    DrawGlasses(P, pos, yaw, b.headR);
    Cap(pos, yaw, V3(-0.19f, P.hip.y + 0.07f, 0.17f), V3(0.19f, P.hip.y + 0.07f, 0.17f), 0.016f, Color{ 20, 18, 18, 255 });
}