#pragma once

#include "raylib.h"
#include <cmath>

// =========================================================
// 1. MATEMATIKA VEKTOR & TRANSFORMASI BONE 3D
// =========================================================
inline Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
inline Vector3 VAdd(Vector3 a, Vector3 b) { return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 VSub(Vector3 a, Vector3 b) { return V3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 VMul(Vector3 a, float s) { return V3(a.x * s, a.y * s, a.z * s); }
inline float VDot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vector3 VCross(Vector3 a, Vector3 b)
{
    return V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
inline float VLen(Vector3 a) { return sqrtf(VDot(a, a)); }
inline Vector3 VNorm(Vector3 a)
{
    float l = VLen(a);
    return l > 1e-6f ? VMul(a, 1.0f / l) : V3(0.0f, 1.0f, 0.0f);
}

inline float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline Vector3 VLerp(Vector3 a, Vector3 b, float t)
{
    return V3(Lerpf(a.x, b.x, t), Lerpf(a.y, b.y, t), Lerpf(a.z, b.z, t));
}

inline float WrapAngle(float a)
{
    const float twoPi = 6.2831853f;
    return a - twoPi * roundf(a / twoPi);
}

// Orientasi 3D lokal (Right, Up, Forward) agar wajah & tubuh bisa miring/menoleh presisi
struct Basis3D {
    Vector3 pos;
    Vector3 right;
    Vector3 up;
    Vector3 forward;
};

inline Basis3D MakeBasis(Vector3 pos, float yaw, float pitch, float roll)
{
    float cy = cosf(yaw), sy = sinf(yaw);
    float cp = cosf(pitch), sp = sinf(pitch);
    float cr = cosf(roll), sr = sinf(roll);

    Vector3 fwd = V3(sy * cp, -sp, cy * cp);
    Vector3 rgt = V3(cy * cr + sy * sp * sr, cp * sr, -sy * cr + cy * sp * sr);
    Vector3 up  = VCross(fwd, rgt);
    return Basis3D{ pos, VNorm(rgt), VNorm(up), VNorm(fwd) };
}

inline Vector3 BasisPoint(const Basis3D& b, float x, float y, float z)
{
    return VAdd(b.pos, VAdd(VMul(b.right, x), VAdd(VMul(b.up, y), VMul(b.forward, z))));
}

inline Vector3 BasisDir(const Basis3D& b, float x, float y, float z)
{
    return VNorm(VAdd(VMul(b.right, x), VAdd(VMul(b.up, y), VMul(b.forward, z))));
}

inline Vector3 RotateYaw(Vector3 o, float yaw)
{
    float c = cosf(yaw), s = sinf(yaw);
    return V3(o.x * c + o.z * s, o.y, -o.x * s + o.z * c);
}

inline Vector3 WorldPoint(Vector3 pos, Vector3 offset, float yaw)
{
    return VAdd(pos, RotateYaw(offset, yaw));
}

// Inverse Kinematics 2 Tulang (Paha-Betis & Lengan-Siku)
inline Vector3 SolveTwoBoneIK(Vector3 root, Vector3& target, float l1, float l2, Vector3 bendDir)
{
    Vector3 d = VSub(target, root);
    float dist = VLen(d);
    float maxD = l1 + l2 - 0.002f;
    if (dist > maxD) {
        d = VMul(VNorm(d), maxD);
        dist = maxD;
        target = VAdd(root, d);
    }
    if (dist < 0.001f) return VAdd(root, V3(0.0f, -l1, 0.0f));

    Vector3 dir = VMul(d, 1.0f / dist);
    float a = (l1 * l1 - l2 * l2 + dist * dist) / (2.0f * dist);
    float h2 = l1 * l1 - a * a;
    float h = h2 > 0.0f ? sqrtf(h2) : 0.0f;
    Vector3 base = VAdd(root, VMul(dir, a));
    Vector3 perp = VSub(bendDir, VMul(dir, VDot(bendDir, dir)));
    perp = VNorm(perp);
    return VAdd(base, VMul(perp, h));
}

// =========================================================
// 2. SISTEM MULTI-TOUCH ANDROID (BISA LARI + TOMBOL SEKALIGUS)
// =========================================================
struct MultiTouchState {
    int prevIds[10];
    int prevCount;
};

inline void InitMultiTouch(MultiTouchState& mt)
{
    mt.prevCount = 0;
    for (int i = 0; i < 10; i++) mt.prevIds[i] = -1;
}

inline bool WasTouchIdDown(const MultiTouchState& mt, int id)
{
    for (int i = 0; i < mt.prevCount; i++) {
        if (mt.prevIds[i] == id) return true;
    }
    return false;
}

// Mendeteksi ketukan jari baru pada tombol walaupun jempol kiri sedang menahan analog lari
inline bool IsButtonJustTapped(const MultiTouchState& mt, Rectangle rect)
{
    int count = GetTouchPointCount();
    for (int i = 0; i < count && i < 10; i++) {
        int id = GetTouchPointId(i);
        Vector2 tp = GetTouchPosition(i);
        if (!WasTouchIdDown(mt, id) && CheckCollisionPointRec(tp, rect)) {
            return true;
        }
    }
    if ( count == 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect)) {
        return true;
    }
    return false;
}

inline void EndMultiTouchFrame(MultiTouchState& mt)
{
    int count = GetTouchPointCount();
    mt.prevCount = count < 10 ? count : 10;
    for (int i = 0; i < mt.prevCount; i++) {
        mt.prevIds[i] = GetTouchPointId(i);
    }
}

// =========================================================
// 3. SISTEM POSE & ANIMASI BIOMEKANIKA WANITA
// =========================================================
struct FemalePose {
    Vector3 pelvis;
    Vector3 waist;
    Vector3 chest;
    Vector3 neckBase;
    Vector3 headCenter;
    float headPitch;
    float headYawOff;
    float headRoll;
    float torsoLean;
    float spineTwist;
    float hipSwayX;
    float hipTilt;

    Vector3 lShoulder, rShoulder;
    Vector3 lElbow, rElbow;
    Vector3 lWrist, rWrist;
    Vector3 lHand, rHand;

    Vector3 lHip, rHip;
    Vector3 lKnee, rKnee;
    Vector3 lAnkle, rAnkle;
    Vector3 lToe, rToe;

    Vector3 hairFlow;
    Vector3 chainFlow;
    float bustBounce;
};

struct AnimState {
    float time;
    float phase;
    float moveBlend;
    float runBlend;
    float crouchBlend;
    float slideBlend;
    float airBlend;
    float landSquash;
    float jumpVelRatio;
    float breath;
    float blinkT;
    float blinkTimer;
    float blinkNext;

    Vector3 hairSpring;
    Vector3 hairVel;
    float chestSpring;
    float chestVel;
};

inline void InitAnim(AnimState& A)
{
    A.time = 0.0f;
    A.phase = 0.0f;
    A.moveBlend = 0.0f;
    A.runBlend = 0.0f;
    A.crouchBlend = 0.0f;
    A.slideBlend = 0.0f;
    A.airBlend = 0.0f;
    A.landSquash = 0.0f;
    A.jumpVelRatio = 0.0f;
    A.breath = 0.0f;
    A.blinkT = -1.0f;
    A.blinkTimer = 0.0f;
    A.blinkNext = 2.6f;
    A.hairSpring = V3(0.0f, 0.0f, 0.0f);
    A.hairVel = V3(0.0f, 0.0f, 0.0f);
    A.chestSpring = 0.0f;
    A.chestVel = 0.0f;
}

inline float BlinkAmount(const AnimState& A)
{
    if (A.blinkT < 0.0f) return 0.0f;
    const float half = 0.075f;
    float v = A.blinkT < half ? A.blinkT / half : (0.15f - A.blinkT) / half;
    return Clamp01(v);
}

inline void UpdateAnim(AnimState& A, float dt, float speedNorm, bool moving,
                       bool sprinting, bool crouching, bool sliding,
                       bool grounded, float velY, float yawRate)
{
    const float twoPi = 6.2831853f;
    A.time += dt;

    float targetMove = (moving && grounded && !sliding) ? 1.0f : 0.0f;
    A.moveBlend = Lerpf(A.moveBlend, targetMove, fminf(dt * 9.0f, 1.0f));

    float targetRun = (sprinting && moving && !crouching && !sliding) ? 1.0f : 0.0f;
    A.runBlend = Lerpf(A.runBlend, targetRun, fminf(dt * 8.5f, 1.0f));

    float targetCrouch = (crouching && !sliding && grounded) ? 1.0f : 0.0f;
    A.crouchBlend = Lerpf(A.crouchBlend, targetCrouch, fminf(dt * 10.0f, 1.0f));

    float targetSlide = sliding ? 1.0f : 0.0f;
    A.slideBlend = Lerpf(A.slideBlend, targetSlide, fminf(dt * 14.0f, 1.0f));

    float targetAir = !grounded ? 1.0f : 0.0f;
    A.airBlend = Lerpf(A.airBlend, targetAir, fminf(dt * 15.0f, 1.0f));
    A.jumpVelRatio = Clampf(velY / 6.5f, -1.0f, 1.0f);

    if (A.landSquash > 0.001f) {
        A.landSquash = Lerpf(A.landSquash, 0.0f, fminf(dt * 10.0f, 1.0f));
    }

    // Frekuensi langkah kaki yang halus dan responsif
    float freq = 3.4f + 6.2f * speedNorm + 4.2f * A.runBlend - 1.6f * A.crouchBlend;
    if (grounded && !sliding && moving) {
        A.phase += dt * freq;
        if (A.phase > twoPi) A.phase -= twoPi;
    }

    A.breath = sinf(A.time * (2.2f + A.runBlend * 2.8f)) * (0.008f + 0.006f * A.runBlend);

    // Fisika Pegas Rambut & Rantai Kacamata
    float windX = Clampf(-yawRate * 0.045f, -0.16f, 0.16f) + cosf(A.phase) * 0.025f * A.moveBlend;
    float windY = Clampf(-velY * 0.035f, -0.10f, 0.18f) + fabsf(sinf(A.phase * 2.0f)) * (0.03f + 0.04f * A.runBlend) * A.moveBlend;
    float windZ = -(0.05f * speedNorm + 0.16f * A.runBlend + 0.28f * A.slideBlend + 0.08f * A.airBlend);

    Vector3 targetHair = V3(windX, windY, windZ);
    Vector3 hForce = VMul(VSub(targetHair, A.hairSpring), 90.0f);
    A.hairVel = VAdd(A.hairVel, VMul(hForce, dt));
    A.hairVel = VMul(A.hairVel, fmaxf(0.0f, 1.0f - dt * 9.0f));
    A.hairSpring = VAdd(A.hairSpring, VMul(A.hairVel, dt));

    // Fisika Sekunder Tubuh
    float targetChest = -sinf(A.phase * 2.0f) * (0.010f + 0.016f * A.runBlend) * A.moveBlend
                        - A.landSquash * 0.025f + Clampf(-velY * 0.004f, -0.02f, 0.02f);
    float cForce = (targetChest - A.chestSpring) * 130.0f;
    A.chestVel = (A.chestVel + cForce * dt) * fmaxf(0.0f, 1.0f - dt * 11.0f);
    A.chestSpring += A.chestVel * dt;

    // Kedipan Mata
    if (A.blinkT >= 0.0f) {
        A.blinkT += dt;
        if (A.blinkT >= 0.15f) {
            A.blinkT = -1.0f;
            A.blinkTimer = 0.0f;
            A.blinkNext = GetRandomValue(24, 52) / 10.0f;
        }
    } else {
        A.blinkTimer += dt;
        if (A.blinkTimer >= A.blinkNext) A.blinkT = 0.0f;
    }
}

inline void ComputeFemalePose(FemalePose& P, const AnimState& A)
{
    float s = sinf(A.phase);
    float c = cosf(A.phase);
    float mv = A.moveBlend;
    float rn = A.runBlend;
    float cr = A.crouchBlend;
    float sl = A.slideBlend;
    float air = A.airBlend;

    // Idle bernapas & tumpuan pinggul natural (Contraposto)
    float idleSwayX = sinf(A.time * 1.3f) * 0.010f * (1.0f - mv);
    float walkSwayX = cosf(A.phase) * (0.032f - 0.008f * rn) * mv;
    P.hipSwayX = (idleSwayX + walkSwayX) * (1.0f - sl) * (1.0f - air);
    P.hipTilt  = sinf(A.phase) * (0.055f + 0.02f * rn) * mv * (1.0f - sl);

    // Tinggi Pinggul (Berdiri: 0.96m, Jongkok: 0.64m, Slide: 0.34m)
    float vertBob = -fabsf(cosf(A.phase)) * (0.026f + 0.038f * rn) * mv;
    float hipY = 0.96f - 0.30f * cr - 0.60f * sl - 0.13f * A.landSquash + vertBob;
    float hipZ = -0.06f * cr - 0.12f * sl;

    P.torsoLean  = 0.04f * mv + 0.18f * rn + 0.22f * cr - 0.38f * sl + 0.08f * air;
    P.spineTwist = -s * (0.12f + 0.14f * rn) * mv * (1.0f - sl);

    P.pelvis = V3(P.hipSwayX, hipY, hipZ);
    P.waist  = V3(P.hipSwayX * 0.55f, hipY + 0.17f - 0.03f * cr - 0.06f * sl, hipZ + P.torsoLean * 0.38f);
    P.chest  = V3(P.hipSwayX * 0.20f, hipY + 0.38f - 0.06f * cr - 0.12f * sl + A.breath, hipZ + P.torsoLean * 0.85f);
    P.neckBase = V3(0.0f, P.chest.y + 0.14f, P.chest.z + P.torsoLean * 0.18f);
    P.headCenter = V3(0.0f, P.neckBase.y + 0.15f, P.neckBase.z + 0.015f);

    P.headPitch  = -P.torsoLean * 0.45f + sinf(A.time * 1.5f) * 0.02f;
    P.headYawOff = -P.spineTwist * 0.45f;
    P.headRoll   = -P.hipSwayX * 0.65f;
    P.bustBounce = A.chestSpring;

    // Bahu & Sendi Pinggul
    const float shWidth = 0.165f;
    const float hipWidth = 0.118f;
    float shTwistZ = sinf(P.spineTwist) * shWidth;
    P.lShoulder = V3(P.chest.x - shWidth, P.chest.y + 0.065f, P.chest.z + shTwistZ);
    P.rShoulder = V3(P.chest.x + shWidth, P.chest.y + 0.065f, P.chest.z - shTwistZ);

    float hipTwistZ = -shTwistZ * 0.45f;
    P.lHip = V3(P.pelvis.x - hipWidth, hipY - P.hipTilt * 0.25f, hipZ + hipTwistZ);
    P.rHip = V3(P.pelvis.x + hipWidth, hipY + P.hipTilt * 0.25f, hipZ - hipTwistZ);

    // --- GERAKAN KAKI FEMININ (Jalan, Lari Cepat, Jongkok, Lompat Lari, Slide) ---
    float strideZ = (0.31f + 0.27f * rn - 0.12f * cr) * mv;
    float lPhaseLift = fmaxf(0.0f, c);
    float rPhaseLift = fmaxf(0.0f, -c);
    float liftHeight = (0.11f + 0.17f * rn) * mv;

    // Saat berjalan/lari, telapak kaki wanita menapak sedikit lebih rapat ke garis tengah (catwalk alignment)
    float footStanceX = Lerpf(0.095f, 0.065f, mv);
    Vector3 lAnkleGround = V3(-footStanceX, 0.085f + lPhaseLift * liftHeight,
                              s * strideZ - (lPhaseLift > 0.0f ? 0.07f * rn : 0.0f));
    Vector3 rAnkleGround = V3( footStanceX, 0.085f + rPhaseLift * liftHeight,
                             -s * strideZ - (rPhaseLift > 0.0f ? 0.07f * rn : 0.0f));

    // Pose Lompat Dinamis (Satu lutut terangkat atletis, satu kaki di belakang)
    float jUp = Clamp01(A.jumpVelRatio * 0.5f + 0.5f);
    Vector3 lAnkleAir = V3(-0.085f, 0.28f + 0.10f * jUp,  0.16f + 0.10f * rn);
    Vector3 rAnkleAir = V3( 0.085f, 0.16f + 0.06f * jUp, -0.22f - 0.08f * rn);

    // Pose Tactical Slide (Kaki kiri meluncur lurus di depan, kaki kanan ditekuk rapat di bawah)
    Vector3 lAnkleSlide = V3(-0.06f, 0.09f,  0.72f);
    Vector3 rAnkleSlide = V3( 0.16f, 0.12f, -0.16f);

    Vector3 lAnkleT = VLerp(VLerp(lAnkleGround, lAnkleAir, air), lAnkleSlide, sl);
    Vector3 rAnkleT = VLerp(VLerp(rAnkleGround, rAnkleAir, air), rAnkleSlide, sl);

    P.lKnee = SolveTwoBoneIK(P.lHip, lAnkleT, 0.43f, 0.42f, V3(-0.02f, 0.10f, 1.0f));
    P.rKnee = SolveTwoBoneIK(P.rHip, rAnkleT, 0.43f, 0.42f, V3( 0.05f + 0.45f * sl, 0.10f, 1.0f));
    P.lAnkle = lAnkleT;
    P.rAnkle = rAnkleT;

    float lToePitch = (s < 0.0f && mv > 0.1f) ? -0.025f : 0.0f;
    float rToePitch = (-s < 0.0f && mv > 0.1f) ? -0.025f : 0.0f;
    P.lToe = VAdd(P.lAnkle, V3(0.0f, -0.055f + lToePitch, 0.135f));
    P.rToe = VAdd(P.rAnkle, V3(0.0f, -0.055f + rToePitch, 0.135f));

    // --- GERAKAN LENGAN & TANGAN ANGGUN ---
    float armSwing = (0.20f + 0.26f * rn) * mv;
    float elbowBendY = 0.05f + 0.16f * rn + 0.10f * cr;
    float wristBaseY = P.chest.y - 0.33f + elbowBendY + fabsf(s) * 0.035f * mv;

    Vector3 lWristGround = V3(-(shWidth + 0.045f - 0.03f * rn), wristBaseY,
                              P.chest.z + 0.05f + 0.12f * rn - s * armSwing);
    Vector3 rWristGround = V3( (shWidth + 0.045f - 0.03f * rn), wristBaseY,
                              P.chest.z + 0.05f + 0.12f * rn + s * armSwing);

    Vector3 lWristAir = V3(-(shWidth + 0.14f), P.chest.y + 0.04f, P.chest.z - 0.08f);
    Vector3 rWristAir = V3( (shWidth + 0.14f), P.chest.y + 0.10f, P.chest.z + 0.10f);

    Vector3 lWristSlide = V3(-(shWidth + 0.18f), hipY - 0.18f, -0.18f);
    Vector3 rWristSlide = V3( (shWidth + 0.10f), hipY + 0.10f,  0.26f);

    Vector3 lWristT = VLerp(VLerp(lWristGround, lWristAir, air), lWristSlide, sl);
    Vector3 rWristT = VLerp(VLerp(rWristGround, rWristAir, air), rWristSlide, sl);

    P.lElbow = SolveTwoBoneIK(P.lShoulder, lWristT, 0.26f, 0.24f, V3(-0.30f, -0.25f, -1.0f));
    P.rElbow = SolveTwoBoneIK(P.rShoulder, rWristT, 0.26f, 0.24f, V3( 0.30f, -0.25f, -1.0f));
    P.lWrist = lWristT;
    P.rWrist = rWristT;

    Vector3 lArmDir = VNorm(VSub(P.lWrist, P.lElbow));
    Vector3 rArmDir = VNorm(VSub(P.rWrist, P.rElbow));
    P.lHand = VAdd(P.lWrist, VMul(lArmDir, 0.065f));
    P.rHand = VAdd(P.rWrist, VMul(rArmDir, 0.065f));

    P.hairFlow  = A.hairSpring;
    P.chainFlow = VMul(A.hairSpring, 0.72f);
}