#pragma once

#include "raylib.h"
#include <cmath>

struct Pose {
    Vector3 hip;
    Vector3 chest;
    Vector3 head;
    Vector3 lShoulder;
    Vector3 rShoulder;
    Vector3 lElbow;
    Vector3 rElbow;
    Vector3 lHand;
    Vector3 rHand;
    Vector3 lHipJ;
    Vector3 rHipJ;
    Vector3 lKnee;
    Vector3 rKnee;
    Vector3 lFoot;
    Vector3 rFoot;
};

struct Body {
    float shoulderW;
    float hipW;
    float headR;
};

struct Style {
    Color skin;
    Color top;
    Color bottom;
    Color skirtC;
    Color hair;
    Color shoe;
    Color lip;
    Color hand;
    float hairLen;
    bool skirt;
    bool boots;
};

struct AnimState {
    float time;
    float phase;
    float amp;
    float breath;
    float blinkT;
    float blinkTimer;
    float blinkNext;
};

inline void InitAnim(AnimState& A)
{
    A.time = 0.0f;
    A.phase = 0.0f;
    A.amp = 0.0f;
    A.breath = 0.0f;
    A.blinkT = -1.0f;
    A.blinkTimer = 0.0f;
    A.blinkNext = 3.0f;
}

inline void UpdateAnim(AnimState& A, float dt, float speedNorm, bool moving)
{
    const float twoPi = 6.2831853f;
    A.time += dt;
    float target = moving ? 1.0f : 0.0f;
    A.amp += (target - A.amp) * fminf(dt * 6.0f, 1.0f);
    A.phase += dt * (2.5f + 7.0f * speedNorm);
    if (A.phase > twoPi) A.phase -= twoPi;
    A.breath = sinf(A.time * 1.9f) * 0.012f;

    if (A.blinkT >= 0.0f) {
        A.blinkT += dt;
        if (A.blinkT >= 0.18f) {
            A.blinkT = -1.0f;
            A.blinkTimer = 0.0f;
            A.blinkNext = GetRandomValue(25, 45) / 10.0f;
        }
    } else {
        A.blinkTimer += dt;
        if (A.blinkTimer >= A.blinkNext) A.blinkT = 0.0f;
    }
}

inline float BlinkAmount(const AnimState& A)
{
    if (A.blinkT < 0.0f) return 0.0f;
    const float half = 0.09f;
    float v = A.blinkT < half ? A.blinkT / half : (0.18f - A.blinkT) / half;
    return fminf(fmaxf(v, 0.0f), 1.0f);
}

inline Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
inline Vector3 VAdd(Vector3 a, Vector3 b) { return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 VSub(Vector3 a, Vector3 b) { return V3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 VMul(Vector3 a, float s) { return V3(a.x * s, a.y * s, a.z * s); }
inline float VDot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float VLen(Vector3 a) { return sqrtf(VDot(a, a)); }
inline Vector3 VNorm(Vector3 a)
{
    float l = VLen(a);
    return l > 1e-6f ? VMul(a, 1.0f / l) : V3(0.0f, 0.0f, 0.0f);
}

inline Vector3 RotateOffset(Vector3 o, float yaw)
{
    float c = cosf(yaw);
    float s = sinf(yaw);
    return V3(o.x * c + o.z * s, o.y, -o.x * s + o.z * c);
}

inline Vector3 WorldPoint(Vector3 pos, Vector3 offset, float yaw)
{
    return VAdd(pos, RotateOffset(offset, yaw));
}

inline Vector3 SolveTwoBone(Vector3 root, Vector3& target, float l1, float l2, Vector3 bend)
{
    Vector3 d = VSub(target, root);
    float dist = VLen(d);
    float maxD = l1 + l2 - 0.001f;
    if (dist > maxD) {
        d = VMul(VNorm(d), maxD);
        dist = maxD;
        target = VAdd(root, d);
    }
    if (dist < 0.001f) {
        return VAdd(root, V3(0.0f, -l1, 0.0f));
    }
    Vector3 dir = VMul(d, 1.0f / dist);
    float a = (l1 * l1 - l2 * l2 + dist * dist) / (2.0f * dist);
    float h2 = l1 * l1 - a * a;
    float h = h2 > 0.0f ? sqrtf(h2) : 0.0f;
    Vector3 base = VAdd(root, VMul(dir, a));
    Vector3 perp = VSub(bend, VMul(dir, VDot(bend, dir)));
    perp = VNorm(perp);
    return VAdd(base, VMul(perp, h));
}

inline void ComputePose(Pose& P, const AnimState& A, const Body& B)
{
    float s = sinf(A.phase);
    float c = cosf(A.phase);
    float a = A.amp;

    float hipY = 0.95f + cosf(2.0f * A.phase) * 0.018f * a;
    float chestY = hipY + 0.4f + A.breath;

    P.hip = V3(0.0f, hipY, 0.0f);
    P.chest = V3(0.0f, chestY, 0.025f * a);
    P.head = V3(0.0f, chestY + 0.3f + fabsf(s) * 0.012f * a, 0.03f * a);

    float shY = chestY + 0.07f;
    P.lShoulder = V3(-B.shoulderW, shY, 0.0f);
    P.rShoulder = V3(B.shoulderW, shY, 0.0f);

    P.lHipJ = V3(-B.hipW, hipY, 0.0f);
    P.rHipJ = V3(B.hipW, hipY, 0.0f);

    float lLift = fmaxf(0.0f, c);
    float rLift = fmaxf(0.0f, -c);
    Vector3 lFootT = V3(-B.hipW, 0.08f + lLift * 0.1f * a, s * 0.32f * a);
    Vector3 rFootT = V3(B.hipW, 0.08f + rLift * 0.1f * a, -s * 0.32f * a);
    P.lKnee = SolveTwoBone(P.lHipJ, lFootT, 0.44f, 0.43f, V3(0.0f, 0.0f, 1.0f));
    P.rKnee = SolveTwoBone(P.rHipJ, rFootT, 0.44f, 0.43f, V3(0.0f, 0.0f, 1.0f));
    P.lFoot = lFootT;
    P.rFoot = rFootT;

    float hy = 0.98f + fabsf(s) * 0.015f * a;
    Vector3 lHandT = V3(-(B.shoulderW + 0.05f), hy, 0.07f - s * 0.22f * a);
    Vector3 rHandT = V3(B.shoulderW + 0.05f, hy, 0.07f + s * 0.22f * a);
    P.lElbow = SolveTwoBone(P.lShoulder, lHandT, 0.3f, 0.27f, V3(0.0f, -0.3f, -1.0f));
    P.rElbow = SolveTwoBone(P.rShoulder, rHandT, 0.3f, 0.27f, V3(0.0f, -0.3f, -1.0f));
    P.lHand = lHandT;
    P.rHand = rHandT;
}

inline void Cap(Vector3 pos, float yaw, Vector3 a, Vector3 b, float r, Color c)
{
    DrawCapsule(WorldPoint(pos, a, yaw), WorldPoint(pos, b, yaw), r, 12, 8, c);
}

inline void Sph(Vector3 pos, float yaw, Vector3 o, float r, Color c)
{
    DrawSphereEx(WorldPoint(pos, o, yaw), r, 10, 14, c);
}

inline float FrontZ(float r, float x, float y)
{
    return sqrtf(fmaxf(0.0f, r * r - x * x - y * y));
}

inline Vector3 OnFace(Vector3 head, float r, float x, float y)
{
    return V3(head.x + x, head.y + y, head.z + FrontZ(r, x, y) + 0.004f);
}

inline Vector3 EyeCenter(Vector3 head, float r, float side)
{
    float ex = 0.058f * side;
    float ey = 0.012f;
    return OnFace(head, r, ex, ey);
}

inline void DrawHumanoid(const Pose& P, Vector3 pos, float yaw, const Style& st, const Body& B, float blink)
{
    const Color white = Color{ 245, 245, 245, 255 };
    const Color iris = Color{ 55, 40, 30, 255 };
    const float r = B.headR;

    if (st.skirt) {
        DrawCylinderEx(WorldPoint(pos, V3(0.0f, P.hip.y + 0.06f, 0.0f), yaw),
                       WorldPoint(pos, V3(0.0f, 0.5f, 0.0f), yaw), 0.2f, 0.27f, 18, st.skirtC);
    } else {
        Cap(pos, yaw, V3(-B.hipW * 0.5f, P.hip.y, 0.0f), V3(B.hipW * 0.5f, P.hip.y, 0.0f), 0.13f, st.bottom);
    }

    Cap(pos, yaw, P.lHipJ, P.lKnee, 0.085f, st.bottom);
    Cap(pos, yaw, P.lKnee, P.lFoot, 0.07f, st.bottom);
    Cap(pos, yaw, P.rHipJ, P.rKnee, 0.085f, st.bottom);
    Cap(pos, yaw, P.rKnee, P.rFoot, 0.07f, st.bottom);

    if (st.boots) {
        Cap(pos, yaw, P.lKnee, VAdd(P.lFoot, V3(0.0f, 0.02f, 0.02f)), 0.075f, st.shoe);
        Cap(pos, yaw, P.rKnee, VAdd(P.rFoot, V3(0.0f, 0.02f, 0.02f)), 0.075f, st.shoe);
    }
    Cap(pos, yaw, P.lFoot, VAdd(P.lFoot, V3(0.0f, 0.0f, 0.12f)), 0.07f, st.shoe);
    Cap(pos, yaw, P.rFoot, VAdd(P.rFoot, V3(0.0f, 0.0f, 0.12f)), 0.07f, st.shoe);

    Cap(pos, yaw, VAdd(P.hip, V3(0.0f, 0.03f, 0.0f)), P.chest, 0.2f, st.top);

    Cap(pos, yaw, P.lShoulder, P.lElbow, 0.062f, st.top);
    Cap(pos, yaw, P.lElbow, P.lHand, 0.052f, st.top);
    Sph(pos, yaw, P.lHand, 0.058f, st.hand);
    Cap(pos, yaw, P.rShoulder, P.rElbow, 0.062f, st.top);
    Cap(pos, yaw, P.rElbow, P.rHand, 0.052f, st.top);
    Sph(pos, yaw, P.rHand, 0.058f, st.hand);

    Cap(pos, yaw, VAdd(P.chest, V3(0.0f, 0.06f, 0.0f)), VAdd(P.head, V3(0.0f, -0.04f, 0.0f)), 0.055f, st.skin);
    Sph(pos, yaw, P.head, r, st.skin);

    Sph(pos, yaw, VAdd(P.head, V3(0.0f, 0.03f, -0.035f)), r + 0.01f, st.hair);
    if (st.hairLen > 0.01f) {
        Cap(pos, yaw, VAdd(P.head, V3(0.0f, -0.02f, -0.1f)), VAdd(P.head, V3(0.0f, -st.hairLen, -0.12f)), r * 0.8f, st.hair);
    }

    for (int k = -1; k <= 1; k += 2) {
        Vector3 ec = EyeCenter(P.head, r, (float)k);
        Sph(pos, yaw, ec, 0.026f, white);
        Sph(pos, yaw, VAdd(ec, V3(0.0f, 0.0f, 0.012f)), 0.014f, iris);
        if (blink > 0.02f) {
            Sph(pos, yaw, VAdd(ec, V3(0.0f, (1.0f - blink) * 0.07f, 0.006f)), 0.027f, st.skin);
        }
    }

    Cap(pos, yaw, OnFace(P.head, r, -0.022f, -0.072f), OnFace(P.head, r, 0.022f, -0.072f), 0.011f, st.lip);
}

inline Body MaleBody() { return Body{ 0.22f, 0.13f, 0.16f }; }

inline Style MaleStyle()
{
    Style s;
    s.skin = Color{ 230, 190, 150, 255 };
    s.top = Color{ 40, 90, 200, 255 };
    s.bottom = Color{ 40, 40, 60, 255 };
    s.skirtC = Color{ 0, 0, 0, 0 };
    s.hair = Color{ 60, 40, 25, 255 };
    s.shoe = Color{ 25, 25, 25, 255 };
    s.lip = Color{ 170, 110, 100, 255 };
    s.hand = Color{ 230, 190, 150, 255 };
    s.hairLen = 0.0f;
    s.skirt = false;
    s.boots = false;
    return s;
}

inline void DrawMaleCharacter(const Pose& P, Vector3 pos, float yaw, float blink)
{
    DrawHumanoid(P, pos, yaw, MaleStyle(), MaleBody(), blink);
}