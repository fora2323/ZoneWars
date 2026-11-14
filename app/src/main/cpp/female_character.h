#pragma once

#include "character.h"

struct BlinkState {
    float timer;
    float next;
    float t;
};

inline void InitBlink(BlinkState& b)
{
    b.timer = 0.0f;
    b.next = 3.0f;
    b.t = -1.0f;
}

inline void UpdateBlink(BlinkState& b, float dt)
{
    if (b.t >= 0.0f) {
        b.t += dt;
        if (b.t >= 0.18f) {
            b.t = -1.0f;
            b.timer = 0.0f;
            b.next = GetRandomValue(25, 45) / 10.0f;
        }
    } else {
        b.timer += dt;
        if (b.timer >= b.next) {
            b.t = 0.0f;
        }
    }
}

inline float BlinkAmount(const BlinkState& b)
{
    if (b.t < 0.0f) return 0.0f;
    const float half = 0.09f;
    float v = b.t < half ? b.t / half : (0.18f - b.t) / half;
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

inline void Ball(Vector3 pos, float yaw, Vector3 o, float r, Color c)
{
    DrawSphereEx(WorldPoint(pos, o, yaw), r, 8, 10, c);
}

inline void Tube(Vector3 pos, float yaw, Vector3 a, Vector3 b, float ra, float rb, Color c)
{
    DrawCylinderEx(WorldPoint(pos, a, yaw), WorldPoint(pos, b, yaw), ra, rb, 10, c);
}

inline void DrawFemaleCharacter(Vector3 pos, float yaw, float phase, float amp, float breath, float blink)
{
    float s = sinf(phase) * 0.3f * amp;
    float sa = sinf(phase) * 0.22f * amp;
    float bob = fabsf(sinf(phase)) * 0.02f * amp;
    float up = breath;
    float hy = breath + bob;
    float r = 0.002f + blink * 0.03f;

    Color skin = Color{ 240, 218, 200, 255 };
    Color dress = Color{ 150, 22, 34, 255 };
    Color tights = Color{ 30, 26, 30, 255 };
    Color leather = Color{ 22, 20, 22, 255 };
    Color glove = Color{ 25, 25, 28, 255 };
    Color hair = Color{ 18, 16, 20, 255 };
    Color gold = Color{ 212, 175, 55, 255 };
    Color lip = Color{ 170, 40, 55, 255 };
    Color white = Color{ 245, 245, 245, 255 };
    Color iris = Color{ 60, 40, 30, 255 };
    Color metal = Color{ 60, 60, 66, 255 };
    Color belt = Color{ 45, 35, 30, 255 };
    Color black = Color{ 10, 10, 10, 255 };

    CapsulePart(pos, yaw, Vector3{ -0.12f, 0.9f, 0.0f }, Vector3{ -0.12f, 0.5f, s * 0.6f }, 0.075f, tights);
    CapsulePart(pos, yaw, Vector3{ -0.12f, 0.5f, s * 0.6f }, Vector3{ -0.12f, 0.16f, s }, 0.06f, tights);
    Tube(pos, yaw, Vector3{ -0.12f, 0.1f, s }, Vector3{ -0.12f, 0.54f, s * 0.6f }, 0.058f, 0.07f, leather);
    CapsulePart(pos, yaw, Vector3{ -0.12f, 0.06f, s - 0.03f }, Vector3{ -0.12f, 0.06f, s + 0.1f }, 0.05f, leather);

    CapsulePart(pos, yaw, Vector3{ 0.12f, 0.9f, 0.0f }, Vector3{ 0.12f, 0.5f, -s * 0.6f }, 0.075f, tights);
    CapsulePart(pos, yaw, Vector3{ 0.12f, 0.5f, -s * 0.6f }, Vector3{ 0.12f, 0.16f, -s }, 0.06f, tights);
    Tube(pos, yaw, Vector3{ 0.12f, 0.1f, -s }, Vector3{ 0.12f, 0.54f, -s * 0.6f }, 0.058f, 0.07f, leather);
    CapsulePart(pos, yaw, Vector3{ 0.12f, 0.06f, -s - 0.03f }, Vector3{ 0.12f, 0.06f, -s + 0.1f }, 0.05f, leather);

    Tube(pos, yaw, Vector3{ 0.0f, 0.72f, 0.0f }, Vector3{ 0.0f, 1.05f, 0.0f }, 0.2f, 0.18f, dress);
    Tube(pos, yaw, Vector3{ 0.0f, 1.05f, 0.0f }, Vector3{ 0.0f, 1.34f + up, 0.0f }, 0.18f, 0.17f, dress);
    Tube(pos, yaw, Vector3{ 0.0f, 1.3f + up, 0.0f }, Vector3{ 0.0f, 1.5f + up, 0.0f }, 0.1f, 0.095f, dress);

    CapsulePart(pos, yaw, Vector3{ -0.19f, 0.98f, 0.0f }, Vector3{ 0.19f, 0.98f, 0.0f }, 0.022f, belt);
    CapsulePart(pos, yaw, Vector3{ 0.12f, 1.38f + up, 0.15f }, Vector3{ -0.14f, 1.0f, 0.18f }, 0.012f, leather);
    CapsulePart(pos, yaw, Vector3{ -0.2f, 0.72f, -0.08f }, Vector3{ -0.2f, 0.72f, 0.08f }, 0.018f, leather);
    CapsulePart(pos, yaw, Vector3{ -0.2f, 0.8f, 0.0f }, Vector3{ -0.2f, 0.64f, 0.04f }, 0.03f, metal);

    CapsulePart(pos, yaw, Vector3{ -0.2f, 1.33f + up, 0.0f }, Vector3{ -0.24f, 1.0f, -sa * 0.5f }, 0.055f, dress);
    CapsulePart(pos, yaw, Vector3{ -0.24f, 1.0f, -sa * 0.5f }, Vector3{ -0.26f, 0.8f, -sa }, 0.047f, dress);
    Ball(pos, yaw, Vector3{ -0.26f, 0.76f, -sa }, 0.05f, glove);

    CapsulePart(pos, yaw, Vector3{ 0.2f, 1.33f + up, 0.0f }, Vector3{ 0.24f, 1.0f, sa * 0.5f }, 0.055f, dress);
    CapsulePart(pos, yaw, Vector3{ 0.24f, 1.0f, sa * 0.5f }, Vector3{ 0.26f, 0.8f, sa }, 0.047f, dress);
    Ball(pos, yaw, Vector3{ 0.26f, 0.76f, sa }, 0.05f, glove);
    CapsulePart(pos, yaw, Vector3{ 0.26f, 0.76f, sa }, Vector3{ 0.26f, 0.74f, sa + 0.16f }, 0.022f, metal);

    Ball(pos, yaw, Vector3{ 0.0f, 1.62f + hy, 0.0f }, 0.15f, skin);

    Ball(pos, yaw, Vector3{ 0.0f, 1.74f + hy, -0.02f }, 0.16f, hair);
    CapsulePart(pos, yaw, Vector3{ 0.0f, 1.66f + hy, -0.12f }, Vector3{ 0.0f, 1.5f + hy, -0.1f }, 0.12f, hair);
    CapsulePart(pos, yaw, Vector3{ -0.11f, 1.76f + hy, 0.11f }, Vector3{ 0.11f, 1.76f + hy, 0.11f }, 0.05f, hair);

    for (int k = -1; k <= 1; k += 2) {
        float sx = (float)k;
        float ex = 0.055f * sx;

        CapsulePart(pos, yaw, Vector3{ ex - 0.03f, 1.7f + hy, 0.14f }, Vector3{ ex + 0.03f, 1.7f + hy, 0.14f }, 0.007f, hair);
        Ball(pos, yaw, Vector3{ ex, 1.64f + hy, 0.13f }, 0.026f, white);
        Ball(pos, yaw, Vector3{ ex, 1.64f + hy, 0.152f }, 0.015f, iris);
        CapsulePart(pos, yaw, Vector3{ ex - 0.03f, 1.67f + hy, 0.14f }, Vector3{ ex + 0.03f, 1.67f + hy, 0.14f }, 0.006f, black);
        CapsulePart(pos, yaw, Vector3{ ex - 0.03f, 1.666f + hy - r, 0.15f }, Vector3{ ex + 0.03f, 1.666f + hy - r, 0.15f }, r, skin);

        float w = 0.042f;
        float top = 1.67f + hy;
        float bottom = 1.62f + hy;
        float gz = 0.158f;
        CapsulePart(pos, yaw, Vector3{ ex - w, top, gz }, Vector3{ ex + w, top, gz }, 0.0045f, gold);
        CapsulePart(pos, yaw, Vector3{ ex - w, bottom, gz }, Vector3{ ex + w, bottom, gz }, 0.0045f, gold);
        CapsulePart(pos, yaw, Vector3{ ex - w, top, gz }, Vector3{ ex - w, bottom, gz }, 0.0045f, gold);
        CapsulePart(pos, yaw, Vector3{ ex + w, top, gz }, Vector3{ ex + w, bottom, gz }, 0.0045f, gold);

        CapsulePart(pos, yaw, Vector3{ sx * 0.097f, 1.655f + hy, 0.158f }, Vector3{ sx * 0.15f, 1.655f + hy, 0.06f }, 0.003f, gold);
        CapsulePart(pos, yaw, Vector3{ sx * 0.15f, 1.655f + hy, 0.06f }, Vector3{ sx * 0.15f, 1.65f + hy, -0.07f }, 0.003f, gold);

        for (int i = 0; i < 8; i++) {
            float t = i / 7.0f;
            Ball(pos, yaw, Vector3{ sx * (0.15f + 0.05f * t), 1.65f + hy - 0.35f * t, -0.07f + 0.07f * t }, 0.005f, gold);
        }
    }

    CapsulePart(pos, yaw, Vector3{ -0.018f, 1.66f + hy, 0.158f }, Vector3{ 0.018f, 1.66f + hy, 0.158f }, 0.0035f, gold);

    Ball(pos, yaw, Vector3{ 0.0f, 1.6f + hy, 0.15f }, 0.017f, skin);
    CapsulePart(pos, yaw, Vector3{ -0.03f, 1.54f + hy, 0.14f }, Vector3{ 0.03f, 1.54f + hy, 0.14f }, 0.016f, lip);
}