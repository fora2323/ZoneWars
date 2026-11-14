#pragma once
#include "raylib.h"
#include <cmath>

struct Basis3D { Vector3 forward, right, up; };

inline Vector3 VAdd(Vector3 a, Vector3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vector3 VSub(Vector3 a, Vector3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vector3 VMul(Vector3 a, float s) { return { a.x * s, a.y * s, a.z * s }; }
inline float VDot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vector3 VCross(Vector3 a, Vector3 b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
inline float VLen(Vector3 a) { return sqrtf(VDot(a, a)); }
inline Vector3 VNorm(Vector3 a) { float l = VLen(a); return l > 0.0001f ? VMul(a, 1.0f / l) : Vector3{0, 1, 0}; }
inline float Clampf(float v, float min, float max) { return v < min ? min : (v > max ? max : v); }
inline float Clamp01(float v) { return Clampf(v, 0.0f, 1.0f); }
inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline Vector3 VLerp(Vector3 a, Vector3 b, float t) { return VAdd(a, VMul(VSub(b, a), t)); }
inline float WrapAngle(float a) { while (a > M_PI) a -= 2.0f * M_PI; while (a < -M_PI) a += 2.0f * M_PI; return a; }

inline Vector3 RotateYaw(Vector3 v, float yaw) {
    float c = cosf(yaw), s = sinf(yaw);
    return { v.x * c - v.z * s, v.y, v.x * s + v.z * c };
}

inline Basis3D MakeBasis(Vector3 forward) {
    Basis3D b;
    b.forward = VNorm(forward);
    Vector3 up = {0, 1, 0};
    if (fabsf(b.forward.y) > 0.99f) up = {0, 0, 1};
    b.right = VNorm(VCross(up, b.forward));
    b.up = VNorm(VCross(b.forward, b.right));
    return b;
}

inline Vector3 WorldPoint(Basis3D b, Vector3 local, Vector3 origin) {
    return VAdd(origin, VAdd(VAdd(VMul(b.right, local.x), VMul(b.up, local.y)), VMul(b.forward, local.z)));
}

inline Vector3 BasisPoint(Basis3D b, Vector3 local) {
    return VAdd(VAdd(VMul(b.right, local.x), VMul(b.up, local.y)), VMul(b.forward, local.z));
}

inline Vector3 BasisDir(Basis3D b, Vector3 local) {
    return VNorm(BasisPoint(b, local));
}

inline Vector3 TwoBoneIK(Vector3 root, Vector3 target, Vector3 pole, float l1, float l2) {
    Vector3 dir = VSub(target, root);
    float d = VLen(dir);
    float maxL = l1 + l2 - 0.001f;
    if (d > maxL) {
        dir = VMul(dir, maxL / d);
        d = maxL;
        target = VAdd(root, dir);
    }
    if (d < 0.001f) return VAdd(root, {0, l1, 0});
    float cosA = (l1 * l1 + d * d - l2 * l2) / (2.0f * l1 * d);
    cosA = Clampf(cosA, -1.0f, 1.0f);
    float angle = acosf(cosA);
    Vector3 axis = VNorm(VCross(dir, VSub(pole, root)));
    if (VLen(axis) < 0.001f) axis = {1, 0, 0};
    Vector3 k = axis;
    Vector3 v = VNorm(dir);
    float c = cosf(angle), s = sinf(angle);
    Vector3 rotV = VAdd(VAdd(VMul(v, c), VMul(VCross(k, v), s)), VMul(k, VDot(k, v) * (1.0f - c)));
    return VAdd(root, VMul(rotV, l1));
}