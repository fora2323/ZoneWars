#pragma once
#include "vec_math.h"

// Pemain (karakter Sausage Man versi ZoneWars)
struct Player {
    Vector3 pos;          // posisi kaki di tanah (dunia)
    Vector3 vel;
    float yaw;            // arah pandang kamera
    float faceYaw;        // arah hadap karakter
    float pitch;
    int state;            // 0 diam, 1 lari, 2 jongkok, 3 tiarap, 4 di udara
    bool isSpinning;
    float spinAngle;
    double lastJumpTime;
    int jumpCount;
    float walkPhase;      // fase siklus langkah
    float moveAmount;     // 0..1 seberapa cepat bergerak (untuk animasi)
    float crouchBlend;    // 0..1 transisi halus jongkok
    float proneBlend;     // 0..1 transisi halus tiarap
};

const float PI_F = 3.14159265f;
const float PRONE_OFF = 0.33f;

static const Color SKIN_C  = {240, 150, 50, 255};
static const Color SHIRT_C = {245, 245, 245, 255};
static const Color SHORTS_C = {40, 60, 140, 255};
static const Color SHOE_C  = {60, 40, 30, 255};
static const Color EYE_C   = {25, 25, 25, 255};
static const Color MOUTH_C = {150, 30, 30, 255};

// ===== Helper matematika =====
inline Vector3 RotX(Vector3 v, float a) {
    float c = cosf(a), s = sinf(a);
    return { v.x, v.y * c - v.z * s, v.y * s + v.z * c };
}

inline Vector3 VLerp(Vector3 a, Vector3 b, float t) {
    return { Lerpf(a.x, b.x, t), Lerpf(a.y, b.y, t), Lerpf(a.z, b.z, t) };
}

// IK dua segmen (paha-betis). Lutut selalu menekuk ke depan (+z).
inline Vector3 SolveKnee(Vector3 hip, Vector3 ank, float L1, float L2) {
    Vector3 d = VSub(ank, hip);
    float len = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (len < 0.001f) len = 0.001f;
    Vector3 dirN = VMul(d, 1.0f / len);

    float a = (L1 * L1 - L2 * L2 + len * len) / (2.0f * len);
    float h = sqrtf(fmaxf(0.0f, L1 * L1 - a * a));

    Vector3 fwd = {0, 0, 1};
    float dp = fwd.x * dirN.x + fwd.y * dirN.y + fwd.z * dirN.z;
    Vector3 perp = VSub(fwd, VMul(dirN, dp));
    float pl = sqrtf(perp.x * perp.x + perp.y * perp.y + perp.z * perp.z);
    if (pl < 0.001f) perp = {0, 0, 1};
    else perp = VMul(perp, 1.0f / pl);

    return VAdd(VAdd(hip, VMul(dirN, a)), VMul(perp, h));
}

// Titik-titik tubuh dalam ruang model (kaki di y=0, depan = +z)
struct Pose {
    Vector3 hipC, chestB, neck, head, spine;
    Vector3 shL, shR, elL, elR, hnL, hnR;
    Vector3 hipL, hipR, knL, knR, anL, anR;
};

inline Pose BuildPose(const Player& p) {
    Pose q;
    const float c   = p.crouchBlend;
    const float pr  = p.proneBlend;
    const float run = p.moveAmount;
    const float ph  = p.walkPhase;

    // Badan: condong saat jongkok/lari, naik-turun saat lari
    const float lean = c * 0.35f + run * 0.12f;
    const float bob  = fabsf(sinf(ph)) * 0.035f * run;
    const float hipY = Lerpf(0.72f, 0.42f, c) + bob;

    q.spine  = RotX({0, 1, 0}, lean);
    q.hipC   = {0, hipY, 0};
    q.chestB = VAdd(q.hipC, VMul(q.spine, 0.10f));
    q.neck   = VAdd(q.chestB, VMul(q.spine, 0.50f));
    q.head   = VAdd(q.neck, VMul(q.spine, 0.26f));

    // Pundak & lengan
    Vector3 shC = VAdd(q.chestB, VMul(q.spine, 0.42f));
    q.shL = VAdd(shC, {-0.32f, 0, 0});
    q.shR = VAdd(shC, { 0.32f, 0, 0});

    float swL = sinf(ph + PI_F) * 0.85f * run;
    float swR = sinf(ph) * 0.85f * run;
    float aL = Lerpf(-0.15f + swL, 2.6f + 0.25f * sinf(ph + PI_F) * run, pr);
    float aR = Lerpf(-0.15f + swR, 2.6f + 0.25f * sinf(ph) * run, pr);
    float bend = Lerpf(0.7f, 0.2f, pr);

    q.elL = VAdd(q.shL, VMul(RotX({0, -1, 0}, aL), 0.30f));
    q.elR = VAdd(q.shR, VMul(RotX({0, -1, 0}, aR), 0.30f));
    q.hnL = VAdd(q.elL, VMul(RotX({0, -1, 0}, aL - bend), 0.28f));
    q.hnR = VAdd(q.elR, VMul(RotX({0, -1, 0}, aR - bend), 0.28f));

    // Kaki
    const float legX = 0.14f;
    const float zs = c * 0.16f + pr * 0.22f;  // kaki maju saat jongkok/tiarap
    q.hipL = {-legX, hipY, pr * 0.22f};
    q.hipR = { legX, hipY, pr * 0.22f};

    float strideL = sinf(ph + PI_F) * 0.30f * run;
    float strideR = sinf(ph) * 0.30f * run;
    float liftL = fmaxf(0.0f, cosf(ph + PI_F)) * 0.12f * run;
    float liftR = fmaxf(0.0f, cosf(ph)) * 0.12f * run;
    q.anL = {-legX, liftL, zs + strideL};
    q.anR = { legX, liftR, zs + strideR};

    q.knL = SolveKnee(q.hipL, q.anL, 0.37f, 0.37f);
    q.knR = SolveKnee(q.hipR, q.anR, 0.37f, 0.37f);

    return q;
}

// Ruang model -> dunia: efek salto, rebah tiarap, lalu rotasi arah hadap
inline Vector3 ToWorld(const Player& p, Vector3 v) {
    if (p.isSpinning) {
        Vector3 piv = {0, 0.9f, 0};
        v = VAdd(piv, RotX(VSub(v, piv), p.spinAngle));
    }
    if (p.proneBlend > 0.0f) {
        float t = p.proneBlend;
        float ny = Lerpf(v.y, PRONE_OFF - v.z, t);
        float nz = Lerpf(v.z, v.y, t);
        v = {v.x, ny, nz};
    }
    // RotateYaw memakai konvensi terbalik, jadi pakai -faceYaw agar depan karakter = arah jalan
    return VAdd(p.pos, RotateYaw(v, -p.faceYaw));
}

inline void DrawSausageMan(const Player& p) {
    Pose q = BuildPose(p);
    auto W = [&p](Vector3 v) { return ToWorld(p, v); };

    // ===== Bayangan: lingkaran pipih di tanah mengikuti posisi tubuh =====
    float shAlpha = Clampf(1.0f - p.pos.y * 0.35f, 0.15f, 1.0f);
    Color shadowCol = {0, 0, 0, (unsigned char)(120.0f * shAlpha)};
    auto Shadow = [&](Vector3 v, float r) {
        Vector3 w = W(v);
        DrawCylinder({w.x, 0.02f, w.z}, r, r, 0.01f, 12, shadowCol);
    };
    Shadow(q.hipC, 0.30f);
    Shadow(q.chestB, 0.30f);
    Shadow(q.neck, 0.28f);
    Shadow(q.head, 0.30f);
    Shadow(q.hnL, 0.10f);
    Shadow(q.hnR, 0.10f);
    Shadow(q.anL, 0.09f);
    Shadow(q.anR, 0.09f);

    // ===== Kaki =====
    DrawCapsule(W(q.hipL), W(q.knL), 0.10f, 8, 8, SKIN_C);
    DrawCapsule(W(q.knL), W(q.anL), 0.09f, 8, 8, SKIN_C);
    DrawCapsule(W(q.hipR), W(q.knR), 0.10f, 8, 8, SKIN_C);
    DrawCapsule(W(q.knR), W(q.anR), 0.09f, 8, 8, SKIN_C);

    // Celana pendek di paha atas
    DrawCapsule(W(q.hipL), W(VLerp(q.hipL, q.knL, 0.45f)), 0.115f, 8, 8, SHORTS_C);
    DrawCapsule(W(q.hipR), W(VLerp(q.hipR, q.knR, 0.45f)), 0.115f, 8, 8, SHORTS_C);

    // Sepatu (pusatnya 0.07 di atas tanah)
    DrawCapsule(W(VAdd(q.anL, {0, 0.07f, -0.05f})), W(VAdd(q.anL, {0, 0.07f, 0.12f})), 0.07f, 8, 8, SHOE_C);
    DrawCapsule(W(VAdd(q.anR, {0, 0.07f, -0.05f})), W(VAdd(q.anR, {0, 0.07f, 0.12f})), 0.07f, 8, 8, SHOE_C);

    // ===== Badan =====
    DrawCapsule(W(q.hipC), W(q.neck), 0.30f, 10, 10, SKIN_C);
    // Celana pendek (pinggul)
    DrawCapsule(W(VSub(q.hipC, VMul(q.spine, 0.08f))),
                W(VAdd(q.hipC, VMul(q.spine, 0.10f))), 0.325f, 10, 10, SHORTS_C);
    // Kaos putih
    DrawCapsule(W(VAdd(q.hipC, VMul(q.spine, 0.10f))),
                W(VSub(q.neck, VMul(q.spine, 0.08f))), 0.315f, 10, 10, SHIRT_C);

    // ===== Lengan =====
    DrawCapsule(W(q.shL), W(q.elL), 0.075f, 8, 8, SKIN_C);
    DrawCapsule(W(q.elL), W(q.hnL), 0.07f, 8, 8, SKIN_C);
    DrawCapsule(W(q.shR), W(q.elR), 0.075f, 8, 8, SKIN_C);
    DrawCapsule(W(q.elR), W(q.hnR), 0.07f, 8, 8, SKIN_C);
    // Lengan baju putih
    DrawCapsule(W(q.shL), W(VLerp(q.shL, q.elL, 0.5f)), 0.09f, 8, 8, SHIRT_C);
    DrawCapsule(W(q.shR), W(VLerp(q.shR, q.elR, 0.5f)), 0.09f, 8, 8, SHIRT_C);
    // Tangan
    DrawSphereEx(W(q.hnL), 0.09f, 6, 6, SKIN_C);
    DrawSphereEx(W(q.hnR), 0.09f, 6, 6, SKIN_C);

    // ===== Kepala & wajah =====
    DrawSphereEx(W(q.head), 0.30f, 12, 12, SKIN_C);

    Vector3 fwd = {0, -q.spine.z, q.spine.y};   // arah depan wajah
    Vector3 up  = VMul(q.spine, 0.03f);
    Vector3 hf  = VMul(fwd, 0.28f);

    Vector3 eL = VAdd(VAdd(q.head, hf), VAdd({-0.11f, 0, 0}, up));
    Vector3 eR = VAdd(VAdd(q.head, hf), VAdd({ 0.11f, 0, 0}, up));
    DrawSphereEx(W(eL), 0.05f, 6, 6, EYE_C);
    DrawSphereEx(W(eR), 0.05f, 6, 6, EYE_C);
    DrawSphereEx(W(VAdd(eL, {0.02f, 0.02f, 0.03f})), 0.015f, 4, 4, WHITE);
    DrawSphereEx(W(VAdd(eR, {0.02f, 0.02f, 0.03f})), 0.015f, 4, 4, WHITE);

    Vector3 m0 = VAdd(VAdd(q.head, VMul(fwd, 0.29f)), VAdd({-0.07f, -0.09f, 0}, up));
    Vector3 m1 = VAdd(VAdd(q.head, VMul(fwd, 0.29f)), VAdd({ 0.07f, -0.09f, 0}, up));
    DrawCapsule(W(m0), W(m1), 0.018f, 6, 6, MOUTH_C);
}

inline void DrawUIButton(Rectangle r, const char* text, bool active) {
    DrawRectangleRec(r, active ? Color{200, 50, 50, 200} : Color{100, 100, 100, 150});
    DrawRectangleLinesEx(r, 3.0f, WHITE);
    int tw = MeasureText(text, 24);
    DrawText(text, (int)(r.x + r.width / 2 - tw / 2), (int)(r.y + r.height / 2 - 12), 24, WHITE);
}