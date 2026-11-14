#pragma once
#include "raylib.h"
#include <cmath>

inline Vector3 VAdd(Vector3 a, Vector3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vector3 VSub(Vector3 a, Vector3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vector3 VMul(Vector3 a, float s) { return { a.x * s, a.y * s, a.z * s }; }
inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float Clampf(float v, float min, float max) { return v < min ? min : (v > max ? max : v); }

inline Vector3 RotateYaw(Vector3 v, float yaw) {
    float c = cosf(yaw), s = sinf(yaw);
    return { v.x * c - v.z * s, v.y, v.x * s + v.z * c };
}