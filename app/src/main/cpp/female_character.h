#pragma once

#include "character.h"

inline void DrawFemaleCharacter(Vector3 pos, float yaw, float phase, float amp)
{
    float s = sinf(phase) * 0.3f * amp;

    Color skin = Color{ 235, 195, 160, 255 };
    Color top = Color{ 220, 80, 140, 255 };
    Color skirt = Color{ 240, 120, 170, 255 };
    Color hair = Color{ 90, 50, 30, 255 };
    Color shoe = Color{ 60, 30, 40, 255 };

    CapsulePart(pos, yaw, Vector3{ -0.12f, 0.62f, 0.0f }, Vector3{ -0.12f, 0.12f, s }, 0.085f, skin);
    CapsulePart(pos, yaw, Vector3{ 0.12f, 0.62f, 0.0f }, Vector3{ 0.12f, 0.12f, -s }, 0.085f, skin);
    SpherePart(pos, yaw, Vector3{ -0.12f, 0.1f, s + 0.04f }, 0.1f, shoe);
    SpherePart(pos, yaw, Vector3{ 0.12f, 0.1f, -s + 0.04f }, 0.1f, shoe);

    CapsulePart(pos, yaw, Vector3{ 0.0f, 0.95f, 0.0f }, Vector3{ 0.0f, 0.58f, 0.0f }, 0.26f, skirt);
    CapsulePart(pos, yaw, Vector3{ 0.0f, 0.95f, 0.0f }, Vector3{ 0.0f, 1.42f, 0.0f }, 0.19f, top);

    CapsulePart(pos, yaw, Vector3{ -0.24f, 1.38f, 0.0f }, Vector3{ -0.28f, 0.95f, -s * 0.8f }, 0.06f, top);
    CapsulePart(pos, yaw, Vector3{ 0.24f, 1.38f, 0.0f }, Vector3{ 0.28f, 0.95f, s * 0.8f }, 0.06f, top);
    SpherePart(pos, yaw, Vector3{ -0.28f, 0.9f, -s * 0.8f }, 0.065f, skin);
    SpherePart(pos, yaw, Vector3{ 0.28f, 0.9f, s * 0.8f }, 0.065f, skin);

    SpherePart(pos, yaw, Vector3{ 0.0f, 1.6f, 0.0f }, 0.16f, skin);
    CapsulePart(pos, yaw, Vector3{ 0.0f, 1.66f, -0.04f }, Vector3{ 0.0f, 1.2f, -0.1f }, 0.16f, hair);
    SpherePart(pos, yaw, Vector3{ 0.0f, 1.7f, -0.02f }, 0.17f, hair);
}
