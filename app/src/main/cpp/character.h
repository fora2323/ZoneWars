#pragma once

#include "raylib.h"
#include <cmath>

inline Vector3 RotateOffset(Vector3 o, float yaw)
{
    float c = cosf(yaw);
    float s = sinf(yaw);
    return Vector3{ o.x * c + o.z * s, o.y, -o.x * s + o.z * c };
}

inline Vector3 WorldPoint(Vector3 pos, Vector3 offset, float yaw)
{
    Vector3 r = RotateOffset(offset, yaw);
    return Vector3{ pos.x + r.x, pos.y + r.y, pos.z + r.z };
}

inline void CapsulePart(Vector3 pos, float yaw, Vector3 a, Vector3 b, float radius, Color color)
{
    DrawCapsule(WorldPoint(pos, a, yaw), WorldPoint(pos, b, yaw), radius, 10, 6, color);
}

inline void SpherePart(Vector3 pos, float yaw, Vector3 o, float radius, Color color)
{
    DrawSphere(WorldPoint(pos, o, yaw), radius, color);
}

inline void DrawMaleCharacter(Vector3 pos, float yaw, float phase, float amp)
{
    float s = sinf(phase) * 0.35f * amp;

    Color skin = Color{ 230, 190, 150, 255 };
    Color shirt = Color{ 40, 90, 200, 255 };
    Color pants = Color{ 40, 40, 60, 255 };
    Color hair = Color{ 60, 40, 25, 255 };
    Color shoe = Color{ 25, 25, 25, 255 };

    CapsulePart(pos, yaw, Vector3{ -0.15f, 0.92f, 0.0f }, Vector3{ -0.15f, 0.14f, s }, 0.1f, pants);
    CapsulePart(pos, yaw, Vector3{ 0.15f, 0.92f, 0.0f }, Vector3{ 0.15f, 0.14f, -s }, 0.1f, pants);
    SpherePart(pos, yaw, Vector3{ -0.15f, 0.1f, s + 0.05f }, 0.11f, shoe);
    SpherePart(pos, yaw, Vector3{ 0.15f, 0.1f, -s + 0.05f }, 0.11f, shoe);

    CapsulePart(pos, yaw, Vector3{ 0.0f, 0.92f, 0.0f }, Vector3{ 0.0f, 1.45f, 0.0f }, 0.22f, shirt);

    CapsulePart(pos, yaw, Vector3{ -0.34f, 1.42f, 0.0f }, Vector3{ -0.36f, 0.9f, -s * 0.8f }, 0.08f, shirt);
    CapsulePart(pos, yaw, Vector3{ 0.34f, 1.42f, 0.0f }, Vector3{ 0.36f, 0.9f, s * 0.8f }, 0.08f, shirt);
    SpherePart(pos, yaw, Vector3{ -0.36f, 0.85f, -s * 0.8f }, 0.075f, skin);
    SpherePart(pos, yaw, Vector3{ 0.36f, 0.85f, s * 0.8f }, 0.075f, skin);

    SpherePart(pos, yaw, Vector3{ 0.0f, 1.62f, 0.0f }, 0.17f, skin);
    SpherePart(pos, yaw, Vector3{ 0.0f, 1.7f, -0.02f }, 0.18f, hair);
}
