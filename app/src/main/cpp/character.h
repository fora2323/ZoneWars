#pragma once

#include "raylib.h"
#include <cmath>

struct Pose {
    Vector3 hip;
    Vector3 waist;
    Vector3 chest;
    Vector3 neck;
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
    float torsoLean;
    float spineTwist;
    float hipSway;
    Vector3 hairOffset;
    Vector3 chainOffset;
};

struct Body {
    float shoulderW;
    float chestW;
    float waistW;
    float hipW;
    float headR;
    float legLen1;
    float legLen2;
};

struct Style {
    Color skin;
    Color skinShadow;
    Color top;
    Color topDark;
    Color bottom;
    Color skirtC;
    Color hair;
    Color hairHighlight;
    Color shoe;
    Color lip;
    Color hand;
    Color harness;
    float hairLen;
    bool skirt;
    bool boots;
    bool tacticalHarness;
};

struct AnimState {
    float time;
    float phase;
    float amp;
    float runBlend;      // 0 = jalan, 1 = lari cepat
    float crouchBlend;   // 0 = berdiri, 1 = jongkok
    float slideBlend;    // 0 = normal, 1 = slide
    float jumpBlend;     // 0 = di tanah, 1 = di udara
    float landSquash;    // Efek pegas lutut saat mendarat
    float breath;
    float blinkT;
    float blinkTimer;
    float blinkNext;
    // Fisika sekunder (rambut, rantai kacamata, tubuh)
    Vector3 hairPos;
    Vector3 hairVel;
    float bodyBounce;
    float bodyBounceVel;
};

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

inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

inline void InitAnim(AnimState& A)
{
    A.time = 0.0f;
    A.phase = 0.0f;
    A.amp = 0.0f;
    A.runBlend = 0.0f;
    A.crouchBlend = 0.0f;
    A.slideBlend = 0.0f;
    A.jumpBlend = 0.0f;
    A.landSquash = 0.0f;
    A.breath = 0.0f;
    A.blinkT = -1.0f;
    A.blinkTimer = 0.0f;
    A.blinkNext = 2.8f;
    A.hairPos = V3(0.0f, 0.0f, 0.0f);
    A.hairVel = V3(0.0f, 0.0f, 0.0f);
    A.bodyBounce = 0.0f;
    A.bodyBounceVel = 0.0f;
}

inline void UpdateAnim(AnimState& A, float dt, float speedNorm, bool moving,
                       bool sprinting, bool crouching, bool sliding,
                       bool grounded, float velY, float yawRate)
{
    const float twoPi = 6.2831853f;
    A.time += dt;

    float smooth = fminf(dt * 10.0f, 1.0f);
    float targetAmp = (moving && grounded && !sliding) ? 1.0f : 0.0f;
    A.amp = Lerpf(A.amp, targetAmp, fminf(dt * 8.0f, 1.0f));

    float targetRun = (sprinting && moving && !crouching && !sliding) ? 1.0f : 0.0f;
    A.runBlend = Lerpf(A.runBlend, targetRun, smooth);

    float targetCrouch = (crouching && !sliding && grounded) ? 1.0f : 0.0f;
    A.crouchBlend = Lerpf(A.crouchBlend, targetCrouch, smooth);

    float targetSlide = sliding ? 1.0f : 0.0f;
    A.slideBlend = Lerpf(A.slideBlend, targetSlide, fminf(dt * 12.0f, 1.0f));

    float targetJump = !grounded ? 1.0f : 0.0f;
    A.jumpBlend = Lerpf(A.jumpBlend, targetJump, fminf(dt * 14.0f, 1.0f));

    if (A.landSquash > 0.001f) {
        A.landSquash = Lerpf(A.landSquash, 0.0f, fminf(dt * 9.0f, 1.0f));
    }

    // Kecepatan siklus langkah (jalan, crouch walk, atau lari cepat)
    float stepFreq = 3.2f + 6.5f * speedNorm + 4.5f * A.runBlend - 2.0f * A.crouchBlend;
    if (grounded && !sliding) {
        A.phase += dt * stepFreq;
        if (A.phase > twoPi) A.phase -= twoPi;
    }

    A.breath = sinf(A.time * (2.2f + A.runBlend * 3.0f)) * (0.012f + 0.01f * A.runBlend);

    // Simulasi pegas rambut & rantai kacamata (Spring-Damper Physics)
    float targetHairX = Clamp01(fabsf(yawRate) * 0.25f) * (yawRate > 0.0f ? -0.12f : 0.12f)
                        + sinf(A.phase) * 0.03f * A.amp;
    float targetHairY = Clamp01(-velY * 0.08f) * 0.14f
                        + fabsf(sinf(A.phase * 2.0f)) * (0.035f + 0.045f * A.runBlend) * A.amp;
    float targetHairZ = -(0.06f * speedNorm + 0.14f * A.runBlend + 0.22f * A.slideBlend);

    Vector3 targetHair = V3(targetHairX, targetHairY, targetHairZ);
    Vector3 force = VMul(VSub(targetHair, A.hairPos), 85.0f);
    A.hairVel = VAdd(A.hairVel, VMul(force, dt));
    A.hairVel = VMul(A.hairVel, fmaxf(0.0f, 1.0f - dt * 9.5f));
    A.hairPos = VAdd(A.hairPos, VMul(A.hairVel, dt));

    // Simulasi pantulan tubuh halus (secondary motion)
    float targetBounce = sinf(A.phase * 2.0f) * (0.018f + 0.025f * A.runBlend) * A.amp
                         - A.landSquash * 0.08f;
    float bForce = (targetBounce - A.bodyBounce) * 110.0f;
    A.bodyBounceVel = (A.bodyBounceVel + bForce * dt) * fmaxf(0.0f, 1.0f - dt * 11.0f);
    A.bodyBounce += A.bodyBounceVel * dt;

    // Kedipan mata natural
    if (A.blinkT >= 0.0f) {
        A.blinkT += dt;
        if (A.blinkT >= 0.16f) {
            A.blinkT = -1.0f;
            A.blinkTimer = 0.0f;
            A.blinkNext = GetRandomValue(22, 48) / 10.0f;
        }
    } else {
        A.blinkTimer += dt;
        if (A.blinkTimer >= A.blinkNext) A.blinkT = 0.0f;
    }
}

inline float BlinkAmount(const AnimState& A)
{
    if (A.blinkT < 0.0f) return 0.0f;
    const float half = 0.08f;
    float v = A.blinkT < half ? A.blinkT / half : (0.16f - A.blinkT) / half;
    return Clamp01(v);
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
    float maxD = l1 + l2 - 0.002f;
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
    float run = A.runBlend;
    float cr = A.crouchBlend;
    float sl = A.slideBlend;
    float jp = A.jumpBlend;

    // 1. Posisi Pinggul (Hip) & Badan
    float bobY = cosf(2.0f * A.phase) * (0.022f + 0.032f * run) * a;
    float baseHipY = 0.94f - 0.28f * cr - 0.56f * sl - 0.14f * A.landSquash + bobY;

    P.hipSway = cosf(A.phase) * (0.028f + 0.015f * run) * a * (1.0f - sl);
    P.spineTwist = -s * (0.09f + 0.12f * run) * a * (1.0f - sl);
    P.torsoLean = (0.05f * a + 0.14f * run + 0.16f * cr - 0.24f * sl);

    P.hip = V3(P.hipSway, baseHipY, -0.05f * cr - 0.10f * sl);
    P.waist = V3(P.hipSway * 0.5f, baseHipY + 0.18f - 0.03f * cr - 0.05f * sl, P.hip.z + P.torsoLean * 0.45f);
    P.chest = V3(0.0f, baseHipY + 0.39f - 0.06f * cr - 0.12f * sl + A.breath + A.bodyBounce * 0.5f,
                 P.hip.z + P.torsoLean);
    P.neck = V3(0.0f, P.chest.y + 0.15f, P.chest.z + P.torsoLean * 0.25f);
    P.head = V3(0.0f, P.chest.y + 0.29f + fabsf(s) * 0.012f * a, P.chest.z + P.torsoLean * 0.35f + 0.02f);

    // 2. Bahu & Pinggul Sendi dengan rotasi berlawanan (Counter-Rotation)
    float shY = P.chest.y + 0.06f;
    float twistZ = sinf(P.spineTwist) * B.shoulderW;
    P.lShoulder = V3(-B.shoulderW, shY, P.chest.z + twistZ);
    P.rShoulder = V3(B.shoulderW, shY, P.chest.z - twistZ);

    float hipTwistZ = -twistZ * 0.45f;
    P.lHipJ = V3(P.hip.x - B.hipW, baseHipY, P.hip.z + hipTwistZ);
    P.rHipJ = V3(P.hip.x + B.hipW, baseHipY, P.hip.z - hipTwistZ);

    // 3. Target Kaki (Jalan, Lari, Jongkok, Lompat, Slide)
    float strideZ = (0.32f + 0.24f * run - 0.10f * cr) * a;
    float liftH = (0.11f + 0.14f * run) * a;
    float lLift = fmaxf(0.0f, c) * liftH;
    float rLift = fmaxf(0.0f, -c) * liftH;

    Vector3 lFootWalk = V3(-B.hipW, 0.07f + lLift, s * strideZ - (s < 0.0f ? 0.06f * run : 0.0f));
    Vector3 rFootWalk = V3(B.hipW, 0.07f + rLift, -s * strideZ - (-s < 0.0f ? 0.06f * run : 0.0f));

    // Pose Kaki saat Lompat
    Vector3 lFootJump = V3(-B.hipW - 0.02f, 0.26f, 0.12f);
    Vector3 rFootJump = V3(B.hipW + 0.02f, 0.14f, -0.18f);

    // Pose Kaki saat Slide (Kaki kiri lurus ke depan, kaki kanan ditekuk di belakang)
    Vector3 lFootSlide = V3(-B.hipW * 0.8f, 0.09f, 0.68f);
    Vector3 rFootSlide = V3(B.hipW * 1.2f, 0.12f, -0.18f);

    Vector3 lFootT = V3(
        Lerpf(Lerpf(lFootWalk.x, lFootJump.x, jp), lFootSlide.x, sl),
        Lerpf(Lerpf(lFootWalk.y, lFootJump.y, jp), lFootSlide.y, sl),
        Lerpf(Lerpf(lFootWalk.z, lFootJump.z, jp), lFootSlide.z, sl)
    );
    Vector3 rFootT = V3(
        Lerpf(Lerpf(rFootWalk.x, rFootJump.x, jp), rFootSlide.x, sl),
        Lerpf(Lerpf(rFootWalk.y, rFootJump.y, jp), rFootSlide.y, sl),
        Lerpf(Lerpf(rFootWalk.z, rFootJump.z, jp), rFootSlide.z, sl)
    );

    P.lKnee = SolveTwoBone(P.lHipJ, lFootT, B.legLen1, B.legLen2, V3(-0.05f, 0.1f, 1.0f));
    P.rKnee = SolveTwoBone(P.rHipJ, rFootT, B.legLen1, B.legLen2, V3(0.08f + 0.35f * sl, 0.1f, 1.0f));
    P.lFoot = lFootT;
    P.rFoot = rFootT;

    // 4. Target Tangan (Ayunan Lari, Keseimbangan Lompat, & Slide)
    float armSwingZ = (0.24f + 0.22f * run) * a;
    float handY = P.chest.y - (0.34f - 0.16f * run - 0.08f * cr) + fabsf(s) * 0.04f * a;

    Vector3 lHandWalk = V3(-(B.shoulderW + 0.05f - 0.04f * run), handY, P.chest.z + 0.08f + 0.10f * run - s * armSwingZ);
    Vector3 rHandWalk = V3(B.shoulderW + 0.05f - 0.04f * run, handY, P.chest.z + 0.08f + 0.10f * run + s * armSwingZ);

    Vector3 lHandJump = V3(-(B.shoulderW + 0.16f), P.chest.y + 0.08f, P.chest.z - 0.06f);
    Vector3 rHandJump = V3(B.shoulderW + 0.16f, P.chest.y + 0.12f, P.chest.z + 0.08f);

    Vector3 lHandSlide = V3(-(B.shoulderW + 0.18f), baseHipY - 0.18f, -0.15f);
    Vector3 rHandSlide = V3(B.shoulderW + 0.12f, baseHipY + 0.12f, 0.28f);

    Vector3 lHandT = V3(
        Lerpf(Lerpf(lHandWalk.x, lHandJump.x, jp), lHandSlide.x, sl),
        Lerpf(Lerpf(lHandWalk.y, lHandJump.y, jp), lHandSlide.y, sl),
        Lerpf(Lerpf(lHandWalk.z, lHandJump.z, jp), lHandSlide.z, sl)
    );
    Vector3 rHandT = V3(
        Lerpf(Lerpf(rHandWalk.x, rHandJump.x, jp), rHandSlide.x, sl),
        Lerpf(Lerpf(rHandWalk.y, rHandJump.y, jp), rHandSlide.y, sl),
        Lerpf(Lerpf(rHandWalk.z, rHandJump.z, jp), rHandSlide.z, sl)
    );

    P.lElbow = SolveTwoBone(P.lShoulder, lHandT, 0.28f, 0.26f, V3(-0.25f, -0.3f, -1.0f));
    P.rElbow = SolveTwoBone(P.rShoulder, rHandT, 0.28f, 0.26f, V3(0.25f, -0.3f, -1.0f));
    P.lHand = lHandT;
    P.rHand = rHandT;

    P.hairOffset = A.hairPos;
    P.chainOffset = VMul(A.hairPos, 0.65f);
}

// Helper Render Hemat GPU untuk RAM 2GB (Slices: 8, Rings: 6)
inline void Cap(Vector3 pos, float yaw, Vector3 a, Vector3 b, float r, Color c)
{
    DrawCapsule(WorldPoint(pos, a, yaw), WorldPoint(pos, b, yaw), r, 8, 6, c);
}

inline void Sph(Vector3 pos, float yaw, Vector3 o, float r, Color c)
{
    DrawSphereEx(WorldPoint(pos, o, yaw), r, 8, 10, c);
}

inline void Cyl(Vector3 pos, float yaw, Vector3 a, Vector3 b, float rTop, float rBot, Color c)
{
    DrawCylinderEx(WorldPoint(pos, a, yaw), WorldPoint(pos, b, yaw), rTop, rBot, 12, c);
}

// Sistem Bayangan Real-Time Proyeksi Matahari (Sangat Ringan untuk Mobile)
inline Vector3 ProjectShadowPoint(Vector3 worldPt)
{
    float h = fmaxf(0.0f, worldPt.y);
    return V3(worldPt.x + h * 0.42f, 0.004f, worldPt.z + h * 0.32f);
}

inline void ShadowCap(Vector3 pos, float yaw, Vector3 a, Vector3 b, float r, Color sc)
{
    Vector3 wa = ProjectShadowPoint(WorldPoint(pos, a, yaw));
    Vector3 wb = ProjectShadowPoint(WorldPoint(pos, b, yaw));
    DrawCapsule(wa, wb, r * 0.85f, 6, 4, sc);
}

inline void DrawCharacterShadow(const Pose& P, Vector3 pos, float yaw)
{
    Color shadowCol = Color{ 16, 22, 30, 105 };
    Color contactCol = Color{ 10, 14, 20, 135 };

    // Contact Ambient Occlusion tepat di bawah kaki/badan
    DrawCylinderEx(V3(pos.x, 0.002f, pos.z), V3(pos.x, 0.003f, pos.z), 0.34f, 0.34f, 14, contactCol);

    // Proyeksi kerangka tubuh ke lantai mengikuti gerakan animasi
    ShadowCap(pos, yaw, P.hip, P.head, 0.17f, shadowCol);
    ShadowCap(pos, yaw, P.lShoulder, P.lElbow, 0.06f, shadowCol);
    ShadowCap(pos, yaw, P.lElbow, P.lHand, 0.05f, shadowCol);
    ShadowCap(pos, yaw, P.rShoulder, P.rElbow, 0.06f, shadowCol);
    ShadowCap(pos, yaw, P.rElbow, P.rHand, 0.05f, shadowCol);
    ShadowCap(pos, yaw, P.lHipJ, P.lKnee, 0.08f, shadowCol);
    ShadowCap(pos, yaw, P.lKnee, P.lFoot, 0.07f, shadowCol);
    ShadowCap(pos, yaw, P.rHipJ, P.rKnee, 0.08f, shadowCol);
    ShadowCap(pos, yaw, P.rKnee, P.rFoot, 0.07f, shadowCol);
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
    return OnFace(head, r, 0.054f * side, 0.012f);
}

inline void DrawHumanoid(const Pose& P, Vector3 pos, float yaw, const Style& st, const Body& B, float blink)
{
    const Color white = Color{ 248, 248, 250, 255 };
    const Color iris = Color{ 62, 44, 38, 255 };
    const Color pupil = Color{ 18, 16, 20, 255 };
    const Color brow = st.hair;
    const float r = B.headR;

    // 1. Kaki, Stocking / Celana, & Boots
    Cap(pos, yaw, P.lHipJ, P.lKnee, 0.086f, st.bottom);
    Cap(pos, yaw, P.lKnee, P.lFoot, 0.068f, st.bottom);
    Cap(pos, yaw, P.rHipJ, P.rKnee, 0.086f, st.bottom);
    Cap(pos, yaw, P.rKnee, P.rFoot, 0.068f, st.bottom);

    if (st.boots) {
        Vector3 lBootTop = VAdd(P.lKnee, V3(0.0f, -0.02f, 0.0f));
        Vector3 rBootTop = VAdd(P.rKnee, V3(0.0f, -0.02f, 0.0f));
        Cap(pos, yaw, lBootTop, VAdd(P.lFoot, V3(0.0f, 0.02f, 0.01f)), 0.074f, st.shoe);
        Cap(pos, yaw, rBootTop, VAdd(P.rFoot, V3(0.0f, 0.02f, 0.01f)), 0.074f, st.shoe);
        // Hak sepatu boots (Heels)
        Cap(pos, yaw, VAdd(P.lFoot, V3(0.0f, -0.03f, -0.03f)), VAdd(P.lFoot, V3(0.0f, 0.03f, -0.03f)), 0.028f, st.shoe);
        Cap(pos, yaw, VAdd(P.rFoot, V3(0.0f, -0.03f, -0.03f)), VAdd(P.rFoot, V3(0.0f, 0.03f, -0.03f)), 0.028f, st.shoe);
    }
    Cap(pos, yaw, P.lFoot, VAdd(P.lFoot, V3(0.0f, -0.01f, 0.125f)), 0.066f, st.shoe);
    Cap(pos, yaw, P.rFoot, VAdd(P.rFoot, V3(0.0f, -0.01f, 0.125f)), 0.066f, st.shoe);

    // 2. Rok Dress atau Pinggul Celana
    if (st.skirt) {
        Vector3 skirtTop = VAdd(P.waist, V3(0.0f, -0.02f, 0.0f));
        Vector3 skirtBot = V3((P.lHipJ.x + P.rHipJ.x) * 0.5f + (P.lKnee.x + P.rKnee.x) * 0.15f,
                              P.hip.y - 0.24f,
                              P.hip.z + (P.lKnee.z + P.rKnee.z) * 0.22f);
        Cyl(pos, yaw, skirtTop, skirtBot, B.waistW, B.hipW + 0.085f, st.skirtC);
        Sph(pos, yaw, P.hip, B.hipW + 0.045f, st.skirtC);
    } else {
        Cap(pos, yaw, VAdd(P.lHipJ, V3(0.02f, 0.02f, 0.0f)), VAdd(P.rHipJ, V3(-0.02f, 0.02f, 0.0f)), 0.125f, st.bottom);
    }

    // 3. Torso Berlekuk (Pinggang -> Dada -> Kerah Turtleneck)
    Cyl(pos, yaw, P.hip, P.waist, B.hipW + 0.03f, B.waistW, st.topDark);
    Cyl(pos, yaw, P.waist, P.chest, B.waistW, B.chestW, st.top);
    Cap(pos, yaw, P.lShoulder, P.rShoulder, 0.072f, st.top);

    // 4. Lengan & Tangan / Sarung Tangan
    Cap(pos, yaw, P.lShoulder, P.lElbow, 0.058f, st.top);
    Cap(pos, yaw, P.lElbow, P.lHand, 0.048f, st.topDark);
    Sph(pos, yaw, P.lHand, 0.052f, st.hand);

    Cap(pos, yaw, P.rShoulder, P.rElbow, 0.058f, st.top);
    Cap(pos, yaw, P.rElbow, P.rHand, 0.048f, st.topDark);
    Sph(pos, yaw, P.rHand, 0.052f, st.hand);

    // 5. Leher & Kepala
    Cap(pos, yaw, VAdd(P.chest, V3(0.0f, 0.04f, 0.0f)), P.neck, 0.058f, st.topDark); // Kerah tinggi
    Cap(pos, yaw, P.neck, VAdd(P.head, V3(0.0f, -0.05f, 0.0f)), 0.052f, st.skinShadow);
    Sph(pos, yaw, P.head, r, st.skin);

    // 6. Wajah (Mata, Pupil, Alis, Hidung Kecil, & Bibir)
    for (int k = -1; k <= 1; k += 2) {
        float side = (float)k;
        Vector3 ec = EyeCenter(P.head, r, side);
        Sph(pos, yaw, ec, 0.024f, white);
        Sph(pos, yaw, VAdd(ec, V3(0.0f, 0.0f, 0.011f)), 0.014f, iris);
        Sph(pos, yaw, VAdd(ec, V3(0.0f, 0.0f, 0.017f)), 0.008f, pupil);
        // Alis
        Cap(pos, yaw, OnFace(P.head, r, 0.032f * side, 0.042f),
            OnFace(P.head, r, 0.078f * side, 0.036f), 0.006f, brow);
        if (blink > 0.02f) {
            Sph(pos, yaw, VAdd(ec, V3(0.0f, (1.0f - blink) * 0.05f, 0.006f)), 0.025f, st.skin);
        }
    }
    // Hidung & Bibir
    Sph(pos, yaw, OnFace(P.head, r, 0.0f, -0.022f), 0.012f, st.skinShadow);
    Cap(pos, yaw, OnFace(P.head, r, -0.022f, -0.066f), OnFace(P.head, r, 0.022f, -0.066f), 0.011f, st.lip);

    // 7. Rambut Dasar & Fisika Ayunan Rambut
    Sph(pos, yaw, VAdd(P.head, V3(0.0f, 0.032f, -0.028f)), r + 0.014f, st.hair);
    if (st.hairLen > 0.01f) {
        Vector3 hRoot = VAdd(P.head, V3(0.0f, 0.02f, -0.11f));
        Vector3 hMid = VAdd(P.head, V3(P.hairOffset.x * 0.6f, -st.hairLen * 0.5f + P.hairOffset.y * 0.5f, -0.15f + P.hairOffset.z * 0.6f));
        Vector3 hTip = VAdd(P.head, V3(P.hairOffset.x * 1.2f, -st.hairLen + P.hairOffset.y, -0.17f + P.hairOffset.z * 1.3f));
        Cap(pos, yaw, hRoot, hMid, r * 0.82f, st.hair);
        Cap(pos, yaw, hMid, hTip, r * 0.68f, st.hair);
    }
}

inline Body MaleBody()
{
    return Body{ 0.22f, 0.19f, 0.15f, 0.135f, 0.155f, 0.44f, 0.43f };
}

inline Style MaleStyle()
{
    Style s;
    s.skin = Color{ 232, 194, 158, 255 };
    s.skinShadow = Color{ 205, 165, 132, 255 };
    s.top = Color{ 38, 52, 74, 255 };        // Jaket taktis biru gelap
    s.topDark = Color{ 26, 36, 52, 255 };
    s.bottom = Color{ 36, 40, 48, 255 };     // Celana kargo gelap
    s.skirtC = Color{ 0, 0, 0, 0 };
    s.hair = Color{ 32, 26, 22, 255 };
    s.hairHighlight = Color{ 55, 45, 38, 255 };
    s.shoe = Color{ 22, 24, 28, 255 };
    s.lip = Color{ 175, 118, 105, 255 };
    s.hand = Color{ 35, 38, 42, 255 };       // Sarung tangan taktis
    s.harness = Color{ 24, 26, 30, 255 };
    s.hairLen = 0.0f;
    s.skirt = false;
    s.boots = true;
    s.tacticalHarness = true;
    return s;
}

inline void DrawMaleCharacter(const Pose& P, Vector3 pos, float yaw, float blink)
{
    Body b = MaleBody();
    Style st = MaleStyle();
    DrawCharacterShadow(P, pos, yaw);
    DrawHumanoid(P, pos, yaw, st, b, blink);

    // Detail rompi/harness sabuk pria
    Cap(pos, yaw, VAdd(P.waist, V3(-0.15f, 0.0f, 0.08f)), VAdd(P.waist, V3(0.15f, 0.0f, 0.08f)), 0.025f, st.harness);
    Cap(pos, yaw, VAdd(P.lShoulder, V3(0.05f, 0.0f, 0.06f)), VAdd(P.waist, V3(-0.08f, 0.02f, 0.12f)), 0.022f, st.harness);
    Cap(pos, yaw, VAdd(P.rShoulder, V3(-0.05f, 0.0f, 0.06f)), VAdd(P.waist, V3(0.08f, 0.02f, 0.12f)), 0.022f, st.harness);
}