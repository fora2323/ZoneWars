#pragma once

#include "raylib.h"
#include "rlgl.h"
#include "math_anim_helper.h"

// =========================================================
// PENCAHAYAAN PER-VERTEX RINGAN (STUDIO LIGHTING + RIM + SPECULAR)
// Membuat bentuk wajah, lekuk tubuh, rambut, & boots tampak 3D halus
// tanpa membebani RAM 2GB dengan tekstur besar.
// =========================================================
inline Color ShadeStudio(Color base, Vector3 normal, float specStrength = 0.15f)
{
    // Arah cahaya utama (atas-depan-kanan) & rim light (belakang-kiri)
    const Vector3 lightDir = V3(0.42f, 0.65f, 0.63f);
    const Vector3 viewDir  = V3(0.0f, 0.25f, 0.96f);
    const Vector3 halfVec  = V3(0.22f, 0.48f, 0.84f);

    float ndl = VDot(normal, lightDir);
    float diff = Clamp01(ndl * 0.55f + 0.45f); // Soft half-lambert agar kulit mulus

    // Rim light lembut di tepi siluet tubuh
    float ndv = Clamp01(VDot(normal, viewDir));
    float rim = (1.0f - ndv);
    rim = rim * rim * 0.22f;

    // Specular highlight untuk rambut hitam berkilau, bibir glossy, dan boots kulit
    float ndh = Clamp01(VDot(normal, halfVec));
    float spec = powf(ndh, 16.0f) * specStrength;

    float shade = 0.62f + 0.42f * diff + rim;
    int r = (int)Clampf(base.r * shade + spec * 245.0f, 0.0f, 255.0f);
    int g = (int)Clampf(base.g * shade + spec * 235.0f, 0.0f, 255.0f);
    int b = (int)Clampf(base.b * shade + spec * 240.0f, 0.0f, 255.0f);
    return Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, base.a };
}

// =========================================================
// LOFTED ANATOMICAL MESH (MENYAMBUNGKAN CINCIN ELIPS 3D)
// Digunakan untuk membuat wajah V-shape, leher, dada, pinggang
// hourglass, pinggul, paha, dan sepatu boots tanpa sambungan patah!
// =========================================================
struct LoftSlice {
    Vector3 center;
    float rx;       // Lebar kiri-kanan
    float rz;       // Tebal depan-belakang
    Color color;
    float spec;
};

inline void DrawLoftedMesh(Vector3 rootPos, float rootYaw, const LoftSlice* slices, int sliceCount, int segs = 14)
{
    if (sliceCount < 2) return;
    const float twoPi = 6.2831853f;

    rlBegin(RL_TRIANGLES);
    for (int s = 0; s < sliceCount - 1; s++) {
        const LoftSlice& s0 = slices[s];
        const LoftSlice& s1 = slices[s + 1];

        for (int i = 0; i < segs; i++) {
            float a0 = (float)i / (float)segs * twoPi;
            float a1 = (float)(i + 1) / (float)segs * twoPi;

            float c0 = cosf(a0), sn0 = sinf(a0);
            float c1 = cosf(a1), sn1 = sinf(a1);

            Vector3 p00 = WorldPoint(rootPos, V3(s0.center.x + c0 * s0.rx, s0.center.y, s0.center.z + sn0 * s0.rz), rootYaw);
            Vector3 p01 = WorldPoint(rootPos, V3(s0.center.x + c1 * s0.rx, s0.center.y, s0.center.z + sn1 * s0.rz), rootYaw);
            Vector3 p10 = WorldPoint(rootPos, V3(s1.center.x + c0 * s1.rx, s1.center.y, s1.center.z + sn0 * s1.rz), rootYaw);
            Vector3 p11 = WorldPoint(rootPos, V3(s1.center.x + c1 * s1.rx, s1.center.y, s1.center.z + sn1 * s1.rz), rootYaw);

            float slopeY = (s0.rx - s1.rx) * 0.45f;
            Vector3 n0 = RotateYaw(VNorm(V3(c0 / fmaxf(0.01f, s0.rx), slopeY, sn0 / fmaxf(0.01f, s0.rz))), rootYaw);
            Vector3 n1 = RotateYaw(VNorm(V3(c1 / fmaxf(0.01f, s0.rx), slopeY, sn1 / fmaxf(0.01f, s0.rz))), rootYaw);

            Color col00 = ShadeStudio(s0.color, n0, s0.spec);
            Color col01 = ShadeStudio(s0.color, n1, s0.spec);
            Color col10 = ShadeStudio(s1.color, n0, s1.spec);
            Color col11 = ShadeStudio(s1.color, n1, s1.spec);

            // Segitiga 1 (CCW)
            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col10.r, col10.g, col10.b, col10.a); rlVertex3f(p10.x, p10.y, p10.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);

            // Segitiga 2 (CCW)
            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);
            rlColor4ub(col01.r, col01.g, col01.b, col01.a); rlVertex3f(p01.x, p01.y, p01.z);
        }
    }
    rlEnd();
}

// Menggambar mesh lofted mengikuti orientasi 3D Kepala (Basis3D)
inline void DrawBasisLoft(const Basis3D& B, const LoftSlice* slices, int sliceCount, int segs = 16)
{
    if (sliceCount < 2) return;
    const float twoPi = 6.2831853f;

    rlBegin(RL_TRIANGLES);
    for (int s = 0; s < sliceCount - 1; s++) {
        const LoftSlice& s0 = slices[s];
        const LoftSlice& s1 = slices[s + 1];

        for (int i = 0; i < segs; i++) {
            float a0 = (float)i / (float)segs * twoPi;
            float a1 = (float)(i + 1) / (float)segs * twoPi;

            float c0 = cosf(a0), sn0 = sinf(a0);
            float c1 = cosf(a1), sn1 = sinf(a1);

            Vector3 p00 = BasisPoint(B, s0.center.x + c0 * s0.rx, s0.center.y, s0.center.z + sn0 * s0.rz);
            Vector3 p01 = BasisPoint(B, s0.center.x + c1 * s0.rx, s0.center.y, s0.center.z + sn1 * s0.rz);
            Vector3 p10 = BasisPoint(B, s1.center.x + c0 * s1.rx, s1.center.y, s1.center.z + sn0 * s1.rz);
            Vector3 p11 = BasisPoint(B, s1.center.x + c1 * s1.rx, s1.center.y, s1.center.z + sn1 * s1.rz);

            float slopeY = (s0.rx - s1.rx) * 0.5f;
            Vector3 n0 = BasisDir(B, c0, slopeY, sn0);
            Vector3 n1 = BasisDir(B, c1, slopeY, sn1);

            Color col00 = ShadeStudio(s0.color, n0, s0.spec);
            Color col01 = ShadeStudio(s0.color, n1, s0.spec);
            Color col10 = ShadeStudio(s1.color, n0, s1.spec);
            Color col11 = ShadeStudio(s1.color, n1, s1.spec);

            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col10.r, col10.g, col10.b, col10.a); rlVertex3f(p10.x, p10.y, p10.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);

            rlColor4ub(col00.r, col00.g, col00.b, col00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(col11.r, col11.g, col11.b, col11.a); rlVertex3f(p11.x, p11.y, p11.z);
            rlColor4ub(col01.r, col01.g, col01.b, col01.a); rlVertex3f(p01.x, p01.y, p01.z);
        }
    }
    rlEnd();
}

// Menggambar Anggota Tubuh (Paha, Betis, Lengan) Berpencahayaan Halus
inline void DrawLitLimb(Vector3 rootPos, float rootYaw, Vector3 a, Vector3 b,
                        float rA, float rB, Color colA, Color colB, float spec = 0.2f, int segs = 12)
{
    Vector3 wa = WorldPoint(rootPos, a, rootYaw);
    Vector3 wb = WorldPoint(rootPos, b, rootYaw);
    Vector3 axis = VSub(wb, wa);
    float len = VLen(axis);
    if (len < 1e-5f) return;
    Vector3 up = VMul(axis, 1.0f / len);

    Vector3 ref = fabsf(up.y) < 0.9f ? V3(0.0f, 1.0f, 0.0f) : V3(1.0f, 0.0f, 0.0f);
    Vector3 right = VNorm(VCross(up, ref));
    Vector3 fwd   = VNorm(VCross(right, up));

    const float twoPi = 6.2831853f;
    rlBegin(RL_TRIANGLES);
    for (int i = 0; i < segs; i++) {
        float a0 = (float)i / (float)segs * twoPi;
        float a1 = (float)(i + 1) / (float)segs * twoPi;

        Vector3 n0 = VAdd(VMul(right, cosf(a0)), VMul(fwd, sinf(a0)));
        Vector3 n1 = VAdd(VMul(right, cosf(a1)), VMul(fwd, sinf(a1)));

        Vector3 p00 = VAdd(wa, VMul(n0, rA));
        Vector3 p01 = VAdd(wa, VMul(n1, rA));
        Vector3 p10 = VAdd(wb, VMul(n0, rB));
        Vector3 p11 = VAdd(wb, VMul(n1, rB));

        Color c00 = ShadeStudio(colA, n0, spec);
        Color c01 = ShadeStudio(colA, n1, spec);
        Color c10 = ShadeStudio(colB, n0, spec);
        Color c11 = ShadeStudio(colB, n1, spec);

        rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
        rlColor4ub(c01.r, c01.g, c01.b, c01.a); rlVertex3f(p01.x, p01.y, p01.z);
        rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);

        rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
        rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);
        rlColor4ub(c10.r, c10.g, c10.b, c10.a); rlVertex3f(p10.x, p10.y, p10.z);
    }
    rlEnd();
}

// Bola 3D Elipsoid Berpencahayaan Halus (untuk Payudara, Sendi, Pipi, Ujung Hidung, Mata)
inline void DrawLitEllipsoid(const Basis3D& B, Vector3 localCenter, Vector3 radii, Color baseCol, float spec = 0.2f, int rings = 8, int slices = 12)
{
    const float pi = 3.14159265f;
    const float twoPi = 6.2831853f;

    rlBegin(RL_TRIANGLES);
    for (int r = 0; r < rings; r++) {
        float v0 = (float)r / (float)rings * pi - pi * 0.5f;
        float v1 = (float)(r + 1) / (float)rings * pi - pi * 0.5f;
        float cv0 = cosf(v0), sv0 = sinf(v0);
        float cv1 = cosf(v1), sv1 = sinf(v1);

        for (int s = 0; s < slices; s++) {
            float u0 = (float)s / (float)slices * twoPi;
            float u1 = (float)(s + 1) / (float)slices * twoPi;
            float cu0 = cosf(u0), su0 = sinf(u0);
            float cu1 = cosf(u1), su1 = sinf(u1);

            Vector3 ln00 = V3(cv0 * cu0, sv0, cv0 * su0);
            Vector3 ln01 = V3(cv0 * cu1, sv0, cv0 * su1);
            Vector3 ln10 = V3(cv1 * cu0, sv1, cv1 * su0);
            Vector3 ln11 = V3(cv1 * cu1, sv1, cv1 * su1);

            Vector3 p00 = BasisPoint(B, localCenter.x + ln00.x * radii.x, localCenter.y + ln00.y * radii.y, localCenter.z + ln00.z * radii.z);
            Vector3 p01 = BasisPoint(B, localCenter.x + ln01.x * radii.x, localCenter.y + ln01.y * radii.y, localCenter.z + ln01.z * radii.z);
            Vector3 p10 = BasisPoint(B, localCenter.x + ln10.x * radii.x, localCenter.y + ln10.y * radii.y, localCenter.z + ln10.z * radii.z);
            Vector3 p11 = BasisPoint(B, localCenter.x + ln11.x * radii.x, localCenter.y + ln11.y * radii.y, localCenter.z + ln11.z * radii.z);

            Color c00 = ShadeStudio(baseCol, BasisDir(B, ln00.x, ln00.y, ln00.z), spec);
            Color c01 = ShadeStudio(baseCol, BasisDir(B, ln01.x, ln01.y, ln01.z), spec);
            Color c10 = ShadeStudio(baseCol, BasisDir(B, ln10.x, ln10.y, ln10.z), spec);
            Color c11 = ShadeStudio(baseCol, BasisDir(B, ln11.x, ln11.y, ln11.z), spec);

            rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(c10.r, c10.g, c10.b, c10.a); rlVertex3f(p10.x, p10.y, p10.z);
            rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);

            rlColor4ub(c00.r, c00.g, c00.b, c00.a); rlVertex3f(p00.x, p00.y, p00.z);
            rlColor4ub(c11.r, c11.g, c11.b, c11.a); rlVertex3f(p11.x, p11.y, p11.z);
            rlColor4ub(c01.r, c01.g, c01.b, c01.a); rlVertex3f(p01.x, p01.y, p01.z);
        }
    }
    rlEnd();
}

// Garis 3D Berkilau untuk Bingkai Kacamata Emas, Bulu Mata, & Tali Harness
inline void DrawBasisTube(const Basis3D& B, Vector3 a, Vector3 b, float r, Color col, float spec = 0.5f)
{
    Vector3 wa = BasisPoint(B, a.x, a.y, a.z);
    Vector3 wb = BasisPoint(B, b.x, b.y, b.z);
    Color lit = ShadeStudio(col, B.forward, spec);
    DrawCapsule(wa, wb, r, 6, 4, lit);
}

// =========================================================
// BAYANGAN REAL-TIME PROYEKSI MATAHARI DI LANTAI
// =========================================================
inline Vector3 ProjectFloorShadow(Vector3 worldPt)
{
    float h = fmaxf(0.0f, worldPt.y);
    return V3(worldPt.x + h * 0.38f, 0.003f, worldPt.z + h * 0.28f);
}

inline void DrawShadowBone(Vector3 pos, float yaw, Vector3 a, Vector3 b, float r, Color sc)
{
    Vector3 wa = ProjectFloorShadow(WorldPoint(pos, a, yaw));
    Vector3 wb = ProjectFloorShadow(WorldPoint(pos, b, yaw));
    DrawCapsule(wa, wb, r * 0.85f, 6, 4, sc);
}

inline void DrawFemaleShadow(const FemalePose& P, Vector3 pos, float yaw)
{
    Color shadowCol  = Color{ 14, 18, 26, 95 };
    Color contactCol = Color{ 10, 12, 18, 130 };

    // Contact shadow lembut di telapak sepatu
    Vector3 lFootW = WorldPoint(pos, P.lAnkle, yaw);
    Vector3 rFootW = WorldPoint(pos, P.rAnkle, yaw);
    if (lFootW.y < 0.25f) {
        DrawCylinderEx(V3(lFootW.x, 0.002f, lFootW.z), V3(lFootW.x, 0.003f, lFootW.z), 0.14f, 0.14f, 10, contactCol);
    }
    if (rFootW.y < 0.25f) {
        DrawCylinderEx(V3(rFootW.x, 0.002f, rFootW.z), V3(rFootW.x, 0.003f, rFootW.z), 0.14f, 0.14f, 10, contactCol);
    }

    DrawShadowBone(pos, yaw, P.pelvis, P.headCenter, 0.15f, shadowCol);
    DrawShadowBone(pos, yaw, P.lShoulder, P.lElbow, 0.055f, shadowCol);
    DrawShadowBone(pos, yaw, P.lElbow, P.lHand, 0.045f, shadowCol);
    DrawShadowBone(pos, yaw, P.rShoulder, P.rElbow, 0.055f, shadowCol);
    DrawShadowBone(pos, yaw, P.rElbow, P.rHand, 0.045f, shadowCol);
    DrawShadowBone(pos, yaw, P.lHip, P.lKnee, 0.08f, shadowCol);
    DrawShadowBone(pos, yaw, P.lKnee, P.lAnkle, 0.065f, shadowCol);
    DrawShadowBone(pos, yaw, P.rHip, P.rKnee, 0.08f, shadowCol);
    DrawShadowBone(pos, yaw, P.rKnee, P.rAnkle, 0.065f, shadowCol);
}