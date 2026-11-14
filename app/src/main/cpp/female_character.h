#pragma once

#include "render_helper.h"

// =========================================================
// 1. WAJAH ANIME/SEMI-REALISTIS V-JAW, KACAMATA EMAS, & RAMBUT
// Sesuai Referensi Gambar 2: Kulit Porselen, Mata Almond Eyeliner,
// Bibir Glossy Merah Rose, Kacamata Emas Rantai, & 2 Kuncir Depan
// =========================================================
inline void DrawSculptedHeadAndHair(const FemalePose& P, Vector3 rootPos, float rootYaw, float blink)
{
    const Color skinBase   = Color{ 250, 235, 228, 255 }; // Porselen cerah halus
    const Color skinBlush  = Color{ 244, 198, 196, 255 }; // Rona pipi lembut
    const Color lipUpper   = Color{ 168, 42, 54, 255 };   // Bibir atas rose-crimson
    const Color lipLower   = Color{ 192, 56, 68, 255 };   // Bibir bawah glossy penuh
    const Color eyeWhite   = Color{ 248, 248, 252, 255 };
    const Color eyeShadow  = Color{ 175, 132, 125, 255 }; // Smoky eyeshadow kelopak mata
    const Color irisCol    = Color{ 112, 82, 68, 255 };   // Iris cokelat hazel lembut
    const Color lashCol    = Color{ 18, 14, 18, 255 };    // Eyeliner & bulu mata lentik
    const Color hairBlack  = Color{ 18, 18, 24, 255 };    // Rambut hitam legam berkilau
    const Color hairTieCol = Color{ 12, 12, 15, 255 };
    const Color goldCol    = Color{ 232, 195, 82, 255 };  // Emas kacamata & rantai

    Vector3 worldHeadPos = WorldPoint(rootPos, P.headCenter, rootYaw);
    Basis3D H = MakeBasis(worldHeadPos, rootYaw + P.headYawOff, P.headPitch, P.headRoll);

    // A. KONTUR WAJAH V-SHAPE (Dagu lancip anggun, tulang pipi tirus, dahi mulus)
    LoftSlice headSlices[7] = {
        { V3(0.0f, -0.105f,  0.032f), 0.022f, 0.026f, skinBase,  0.15f }, // Ujung dagu V-shape lancip
        { V3(0.0f, -0.072f,  0.015f), 0.068f, 0.078f, skinBase,  0.12f }, // Garis rahang bawah tirus
        { V3(0.0f, -0.028f,  0.006f), 0.098f, 0.108f, skinBlush, 0.18f }, // Pipi & rona blush
        { V3(0.0f,  0.015f,  0.000f), 0.108f, 0.118f, skinBase,  0.15f }, // Tulang pipi & area mata
        { V3(0.0f,  0.065f, -0.006f), 0.106f, 0.120f, skinBase,  0.15f }, // Dahi
        { V3(0.0f,  0.108f, -0.014f), 0.085f, 0.102f, skinBase,  0.12f }, // Kepala atas
        { V3(0.0f,  0.130f, -0.020f), 0.035f, 0.045f, hairBlack, 0.45f }  // Puncak ubun-ubun
    };
    DrawBasisLoft(H, headSlices, 7, 18);

    // B. HIDUNG MANCUNG & BIBIR GLOSSY
    DrawBasisTube(H, V3(0.0f, 0.018f, 0.114f), V3(0.0f, -0.024f, 0.129f), 0.0085f, skinBase, 0.25f);
    DrawLitEllipsoid(H, V3(0.0f, -0.026f, 0.128f), V3(0.011f, 0.009f, 0.012f), skinBase, 0.30f, 6, 8);

    // Bibir atas (Cupid's bow) & bibir bawah penuh berkilau
    DrawBasisTube(H, V3(-0.022f, -0.056f, 0.108f), V3(0.0f, -0.053f, 0.116f), 0.0065f, lipUpper, 0.55f);
    DrawBasisTube(H, V3( 0.000f, -0.053f, 0.116f), V3(0.022f, -0.056f, 0.108f), 0.0065f, lipUpper, 0.55f);
    DrawLitEllipsoid(H, V3(0.0f, -0.062f, 0.112f), V3(0.021f, 0.0075f, 0.011f), lipLower, 0.75f, 6, 10);
    // Titik kilau bibir (Lip gloss highlight)
    DrawLitEllipsoid(H, V3(0.005f, -0.061f, 0.121f), V3(0.005f, 0.0025f, 0.003f), eyeWhite, 0.9f, 4, 6);

    // C. MATA ALMOND / CAT-EYE + EYELINER + BULU MATA + ALIS
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        float ex = 0.044f * sx;
        float ey = 0.014f;
        float ez = 0.105f;

        // Eyeshadow kelopak atas
        DrawLitEllipsoid(H, V3(ex, ey + 0.006f, ez - 0.003f), V3(0.024f, 0.012f, 0.010f), eyeShadow, 0.2f, 6, 8);

        if (blink < 0.85f) {
            float openScale = 1.0f - blink * 0.9f;
            // Sclera berbentuk almond (lebar horizontal, ramping vertikal)
            DrawLitEllipsoid(H, V3(ex, ey, ez), V3(0.021f, 0.0105f * openScale, 0.009f), eyeWhite, 0.4f, 6, 10);
            // Iris cokelat hazel & pupil hitam
            DrawLitEllipsoid(H, V3(ex, ey, ez + 0.005f), V3(0.0115f, 0.010f * openScale, 0.006f), irisCol, 0.5f, 6, 10);
            DrawLitEllipsoid(H, V3(ex, ey, ez + 0.008f), V3(0.0065f, 0.0065f * openScale, 0.004f), lashCol, 0.6f, 5, 8);
            // Catchlight (Pantulan cahaya putih di mata agar hidup!)
            DrawLitEllipsoid(H, V3(ex - 0.004f * sx, ey + 0.0035f * openScale, ez + 0.011f),
                             V3(0.0032f, 0.0032f, 0.002f), eyeWhite, 1.0f, 4, 6);
        }

        // Garis Eyeliner Winged (Meruncing ke sudut luar seperti referensi)
        float lidDrop = blink * 0.012f;
        Vector3 innerCorner = V3(ex - 0.019f * sx, ey - 0.003f, ez + 0.003f);
        Vector3 midUpper    = V3(ex + 0.002f * sx, ey + 0.010f - lidDrop, ez + 0.007f);
        Vector3 outerWing   = V3(ex + 0.025f * sx, ey + 0.004f - lidDrop * 0.5f, ez - 0.002f);
        Vector3 wingTip     = V3(ex + 0.032f * sx, ey + 0.009f, ez - 0.006f);
        DrawBasisTube(H, innerCorner, midUpper, 0.0028f, lashCol, 0.4f);
        DrawBasisTube(H, midUpper, outerWing, 0.0032f, lashCol, 0.4f);
        DrawBasisTube(H, outerWing, wingTip, 0.0022f, lashCol, 0.4f);

        // Alis melengkung rapi
        Vector3 browIn  = V3(ex - 0.016f * sx, ey + 0.026f, ez + 0.004f);
        Vector3 browMid = V3(ex + 0.006f * sx, ey + 0.032f, ez + 0.002f);
        Vector3 browOut = V3(ex + 0.028f * sx, ey + 0.023f, ez - 0.006f);
        DrawBasisTube(H, browIn, browMid, 0.0035f, lashCol, 0.3f);
        DrawBasisTube(H, browMid, browOut, 0.0026f, lashCol, 0.3f);

        // D. KACAMATA OVAL EMAS + RANTAI EMAS MENJUNTAI (Sesuai Referensi)
        float gx = 0.046f * sx;
        float gy = 0.008f;
        float gz = 0.122f;
        const int gSegs = 12;
        const float twoPi = 6.2831853f;
        for (int i = 0; i < gSegs; i++) {
            float a0 = (float)i / (float)gSegs * twoPi;
            float a1 = (float)(i + 1) / (float)gSegs * twoPi;
            Vector3 p0 = V3(gx + cosf(a0) * 0.032f, gy + sinf(a0) * 0.018f, gz - fabsf(cosf(a0)) * 0.005f);
            Vector3 p1 = V3(gx + cosf(a1) * 0.032f, gy + sinf(a1) * 0.018f, gz - fabsf(cosf(a1)) * 0.005f);
            DrawBasisTube(H, p0, p1, 0.0022f, goldCol, 0.85f);
        }

        // Engsel luar kacamata & Rantai Emas Berayun
        Vector3 hinge = V3(gx + 0.034f * sx, gy + 0.004f, gz - 0.008f);
        Vector3 earPt = V3(0.106f * sx, gy + 0.002f, 0.010f);
        DrawBasisTube(H, hinge, earPt, 0.0020f, goldCol, 0.85f);

        Vector3 prevLink = hinge;
        for (int c = 1; c <= 8; c++) {
            float t = (float)c / 8.0f;
            float drapeZ = -sinf(t * 3.14159f) * 0.028f;
            Vector3 nextLink = V3(
                hinge.x + sx * (0.008f * sinf(t * 3.14f)) + P.chainFlow.x * t,
                hinge.y - t * 0.28f + P.chainFlow.y * t * 0.4f,
                hinge.z + drapeZ - t * 0.035f + P.chainFlow.z * t
            );
            DrawBasisTube(H, prevLink, nextLink, 0.0018f, goldCol, 0.9f);
            DrawLitEllipsoid(H, nextLink, V3(0.0032f, 0.0042f, 0.0032f), goldCol, 0.9f, 4, 6);
            prevLink = nextLink;
        }
    }
    // Jembatan hidung kacamata emas melengkung
    DrawBasisTube(H, V3(-0.014f, 0.012f, 0.123f), V3(0.0f, 0.016f, 0.126f), 0.0022f, goldCol, 0.85f);
    DrawBasisTube(H, V3( 0.000f, 0.016f, 0.126f), V3(0.014f, 0.012f, 0.123f), 0.0022f, goldCol, 0.85f);

    // E. VOLUME RAMBUT HITAM PANJANG, PONI SAMPING, & 2 KUNCIR DEPAN (Sesuai Referensi)
    // Mahkota rambut bervolume di kepala atas & belakang
    DrawLitEllipsoid(H, V3(-0.025f, 0.045f, -0.018f), V3(0.112f, 0.115f, 0.118f), hairBlack, 0.55f, 8, 12);
    DrawLitEllipsoid(H, V3( 0.025f, 0.045f, -0.018f), V3(0.112f, 0.115f, 0.118f), hairBlack, 0.55f, 8, 12);

    // Poni belah tengah/samping menjuntai lembut membingkai wajah & kacamata
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        Vector3 b0 = V3(0.008f * sx, 0.118f, 0.088f);
        Vector3 b1 = V3(0.058f * sx, 0.068f, 0.112f);
        Vector3 b2 = V3(0.096f * sx, -0.005f, 0.086f);
        Vector3 b3 = V3(0.104f * sx, -0.075f, 0.048f);
        DrawBasisTube(H, b0, b1, 0.024f, hairBlack, 0.6f);
        DrawBasisTube(H, b1, b2, 0.020f, hairBlack, 0.6f);
        DrawBasisTube(H, b2, b3, 0.014f, hairBlack, 0.6f);
        // Helaian tipis poni melewati tepi kacamata
        DrawBasisTube(H, V3(0.015f * sx, 0.112f, 0.095f), V3(0.048f * sx, 0.025f, 0.118f), 0.006f, hairBlack, 0.6f);

        // Dua Untai Rambut Depan Panjang dengan Ikatan Hitam di Atas Dada (Ciri Khas Gambar 2)
        Vector3 lockTop = WorldPoint(rootPos, VAdd(P.headCenter, V3(0.095f * sx, -0.04f, 0.03f)), rootYaw);
        Vector3 lockMid = WorldPoint(rootPos, VAdd(P.chest, V3(0.092f * sx + P.hairFlow.x * 0.35f,
                                     0.035f + P.hairFlow.y * 0.25f, 0.125f + P.hairFlow.z * 0.25f)), rootYaw);
        Vector3 lockTip = WorldPoint(rootPos, VAdd(P.chest, V3(0.102f * sx + P.hairFlow.x * 0.75f,
                                     -0.19f + P.hairFlow.y * 0.5f, 0.115f + P.hairFlow.z * 0.55f)), rootYaw);

        Color litHair = ShadeStudio(hairBlack, H.forward, 0.55f);
        DrawCapsule(lockTop, lockMid, 0.022f, 8, 6, litHair);
        // Pita lilitan ikat rambut hitam di dada
        DrawCapsule(VAdd(lockMid, V3(0.0f, 0.018f, 0.0f)), VAdd(lockMid, V3(0.0f, -0.018f, 0.0f)), 0.026f, 8, 6, hairTieCol);
        DrawCapsule(lockMid, lockTip, 0.019f, 8, 6, litHair);
    }

    // Rambut Panjang Belakang Terurai Lebat dengan Fisika Ayunan Halus
    LoftSlice backHair[5] = {
        { VAdd(P.headCenter, V3(0.0f,  0.04f, -0.07f)), 0.115f, 0.055f, hairBlack, 0.55f },
        { VAdd(P.neckBase,   V3(P.hairFlow.x * 0.2f,  0.02f, -0.09f + P.hairFlow.z * 0.2f)), 0.135f, 0.050f, hairBlack, 0.55f },
        { VAdd(P.chest,      V3(P.hairFlow.x * 0.5f,  0.02f + P.hairFlow.y * 0.3f, -0.11f + P.hairFlow.z * 0.5f)), 0.150f, 0.045f, hairBlack, 0.55f },
        { VAdd(P.waist,      V3(P.hairFlow.x * 0.9f,  0.05f + P.hairFlow.y * 0.6f, -0.12f + P.hairFlow.z * 0.9f)), 0.140f, 0.038f, hairBlack, 0.55f },
        { VAdd(P.waist,      V3(P.hairFlow.x * 1.3f, -0.12f + P.hairFlow.y * 1.0f, -0.13f + P.hairFlow.z * 1.3f)), 0.095f, 0.025f, hairBlack, 0.55f }
    };
    DrawLoftedMesh(rootPos, rootYaw, backHair, 5, 12);
}

// =========================================================
// 2. TUBUH HOURGLASS, SWEATER DRESS MERAH MARUN, HARNESS & BOOTS
// Sesuai Referensi Gambar 1 & 2
// =========================================================
inline void DrawFemaleCharacter(const FemalePose& P, Vector3 pos, float yaw, float blink)
{
    const Color skinCol      = Color{ 250, 235, 228, 255 };
    const Color dressBright  = Color{ 148, 22, 38, 255 };   // Merah marun terang (bagian terkena cahaya)
    const Color dressMid     = Color{ 122, 16, 30, 255 };   // Merah marun rajut
    const Color dressDeep    = Color{ 88, 10, 22, 255 };    // Lipatan pinggang & kerah gelap
    const Color stockingCol  = Color{ 32, 28, 34, 255 };    // Stocking hitam sheer
    const Color stockingTop  = Color{ 22, 20, 24, 255 };
    const Color bootLeather  = Color{ 18, 18, 22, 255 };    // Kulit boots hitam mengilap
    const Color leatherGear  = Color{ 24, 22, 26, 255 };    // Harness & holster hitam
    const Color silverBuckle = Color{ 205, 210, 220, 255 };

    // 1. Gambar Bayangan Real-Time di Lantai
    DrawFemaleShadow(P, pos, yaw);

    // 2. TUBUH HOURGLASS MULUS (Dari Rok Mini -> Pinggul -> Pinggang Ramping -> Dada -> Kerah Turtleneck)
    Vector3 hipMid = P.pelvis;
    Vector3 wst    = P.waist;
    Vector3 chs    = P.chest;
    Vector3 nck    = P.neckBase;

    LoftSlice dressSlices[9] = {
        // Hem bawah rok mini rajut (pas di atas paha)
        { V3(hipMid.x, hipMid.y - 0.21f, hipMid.z + (P.lKnee.z + P.rKnee.z) * 0.16f), 0.182f, 0.128f, dressDeep,   0.18f },
        // Panggul bawah & lengkung bokong (Glutes curve)
        { V3(hipMid.x, hipMid.y - 0.08f, hipMid.z - 0.018f),                          0.192f, 0.142f, dressBright, 0.22f },
        // Pinggul utama (Lebar feminin)
        { V3(hipMid.x, hipMid.y + 0.02f, hipMid.z - 0.008f),                          0.180f, 0.126f, dressMid,    0.20f },
        // Pinggang Hourglass Ramping
        { V3(wst.x,    wst.y,            wst.z),                                      0.118f, 0.088f, dressDeep,   0.15f },
        // Rusuk bawah (Ribcage)
        { V3(chs.x,    chs.y - 0.09f,    chs.z),                                      0.138f, 0.098f, dressMid,    0.20f },
        // Dada penuh (Full Chest contour)
        { V3(chs.x,    chs.y + 0.01f,    chs.z + 0.018f),                             0.156f, 0.118f, dressBright, 0.26f },
        // Bahu atas & leher bawah
        { V3(chs.x,    chs.y + 0.09f,    chs.z),                                      0.168f, 0.092f, dressMid,    0.20f },
        // Pangkal kerah turtleneck
        { V3(nck.x,    nck.y - 0.01f,    nck.z),                                      0.062f, 0.058f, dressDeep,   0.18f },
        // Kerah tinggi Turtleneck merah marun
        { V3(nck.x,    nck.y + 0.055f,   nck.z),                                      0.056f, 0.054f, dressBright, 0.22f }
    };
    DrawLoftedMesh(pos, yaw, dressSlices, 9, 18);

    // Leher atas porselen menyambung ke kepala
    DrawLitLimb(pos, yaw, VAdd(nck, V3(0.0f, 0.045f, 0.0f)), P.headCenter, 0.048f, 0.044f, skinCol, skinCol, 0.15f, 12);

    // 3. KONTUR PAYUDARA (BUST) & AKSEN POTONGAN DADA (Sesuai Referensi)
    Basis3D chestBasis = MakeBasis(WorldPoint(pos, chs, yaw), yaw + P.spineTwist * 0.5f, P.torsoLean, 0.0f);
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        DrawLitEllipsoid(chestBasis, V3(0.068f * sx, 0.005f + P.bustBounce, 0.068f),
                         V3(0.074f, 0.068f, 0.072f), dressBright, 0.28f, 8, 12);
    }
    // Kontur bokong belakang (Glutes) agar siluet samping proporsional seperti Gambar 1
    Basis3D pelvisBasis = MakeBasis(WorldPoint(pos, hipMid, yaw), yaw, 0.0f, 0.0f);
    for (int side = -1; side <= 1; side += 2) {
        DrawLitEllipsoid(pelvisBasis, V3(0.065f * (float)side, -0.05f, -0.055f),
                         V3(0.078f, 0.082f, 0.072f), dressMid, 0.22f, 7, 10);
    }

    // 4. LENGAN RAMPING & SARUNG TANGAN KULIT HITAM
    DrawLitLimb(pos, yaw, P.lShoulder, P.lElbow, 0.052f, 0.042f, dressBright, dressMid, 0.20f, 12);
    DrawLitLimb(pos, yaw, P.lElbow, P.lWrist,    0.041f, 0.033f, dressMid, dressDeep,   0.20f, 12);
    DrawLitLimb(pos, yaw, P.lWrist, P.lHand,     0.036f, 0.028f, leatherGear, leatherGear, 0.55f, 10);

    DrawLitLimb(pos, yaw, P.rShoulder, P.rElbow, 0.052f, 0.042f, dressBright, dressMid, 0.20f, 12);
    DrawLitLimb(pos, yaw, P.rElbow, P.rWrist,    0.041f, 0.033f, dressMid, dressDeep,   0.20f, 12);
    DrawLitLimb(pos, yaw, P.rWrist, P.rHand,     0.036f, 0.028f, leatherGear, leatherGear, 0.55f, 10);

    // 5. KAKI JENJANG (STOCKING SHEER + SEPATU BOOTS TINGGI HAK STILETTO)
    // Paha Kiri & Kanan (Stocking hitam sheer dengan kilau halus)
    DrawLitLimb(pos, yaw, P.lHip, P.lKnee, 0.092f, 0.062f, stockingTop, stockingCol, 0.35f, 14);
    DrawLitLimb(pos, yaw, P.rHip, P.rKnee, 0.092f, 0.062f, stockingTop, stockingCol, 0.35f, 14);

    // Sepatu Boots Kulit Setinggi Lutut (Knee-High Leather Boots)
    Vector3 lBootCuff = VLerp(P.lHip, P.lKnee, 0.92f);
    Vector3 rBootCuff = VLerp(P.rHip, P.rKnee, 0.92f);
    DrawLitLimb(pos, yaw, lBootCuff, P.lKnee,  0.068f, 0.065f, bootLeather, bootLeather, 0.65f, 14);
    DrawLitLimb(pos, yaw, P.lKnee,   P.lAnkle, 0.064f, 0.045f, bootLeather, bootLeather, 0.65f, 14);
    DrawLitLimb(pos, yaw, rBootCuff, P.rKnee,  0.068f, 0.065f, bootLeather, bootLeather, 0.65f, 14);
    DrawLitLimb(pos, yaw, P.rKnee,   P.rAnkle, 0.064f, 0.045f, bootLeather, bootLeather, 0.65f, 14);

    // Punggung Sepatu Meruncip & Hak Tinggi (Stiletto Heels)
    DrawLitLimb(pos, yaw, P.lAnkle, P.lToe, 0.046f, 0.028f, bootLeather, bootLeather, 0.70f, 10);
    DrawLitLimb(pos, yaw, P.rAnkle, P.rToe, 0.046f, 0.028f, bootLeather, bootLeather, 0.70f, 10);
    Vector3 lHeelTop = VAdd(P.lAnkle, V3(0.0f, -0.01f, -0.028f));
    Vector3 lHeelBot = VAdd(P.lAnkle, V3(0.0f, -0.082f, -0.028f));
    Vector3 rHeelTop = VAdd(P.rAnkle, V3(0.0f, -0.01f, -0.028f));
    Vector3 rHeelBot = VAdd(P.rAnkle, V3(0.0f, -0.082f, -0.028f));
    DrawLitLimb(pos, yaw, lHeelTop, lHeelBot, 0.016f, 0.009f, bootLeather, bootLeather, 0.70f, 8);
    DrawLitLimb(pos, yaw, rHeelTop, rHeelBot, 0.016f, 0.009f, bootLeather, bootLeather, 0.70f, 8);

    // 6. HARNESS BAHU TAKTIS, SABUK PINGGANG MIRING, & HOLSTER PAHA (Sesuai Referensi Gambar 1)
    DrawBasisTube(chestBasis, V3(-0.095f, 0.065f, 0.095f), V3(0.095f, 0.065f, 0.095f), 0.012f, leatherGear, 0.5f);
    DrawBasisTube(chestBasis, V3(-0.125f, 0.085f, 0.000f), V3(-0.095f, 0.065f, 0.095f), 0.014f, leatherGear, 0.5f);
    DrawBasisTube(chestBasis, V3( 0.125f, 0.085f, 0.000f), V3( 0.095f, 0.065f, 0.095f), 0.014f, leatherGear, 0.5f);
    // Holster bawah ketiak kiri
    DrawBasisTube(chestBasis, V3(-0.145f, 0.01f, 0.02f), V3(-0.145f, -0.08f, 0.04f), 0.022f, leatherGear, 0.5f);

    // Sabuk kulit melingkar miring di pinggang bawah + gesper perak
    DrawBasisTube(pelvisBasis, V3(-0.155f, 0.11f, 0.095f), V3(0.160f, 0.06f, 0.095f), 0.014f, leatherGear, 0.55f);
    DrawBasisTube(pelvisBasis, V3(-0.155f, 0.11f, -0.095f), V3(0.160f, 0.06f, -0.095f), 0.014f, leatherGear, 0.55f);
    DrawLitEllipsoid(pelvisBasis, V3(0.035f, 0.082f, 0.108f), V3(0.018f, 0.014f, 0.008f), silverBuckle, 0.9f, 5, 8);

    // Strap ganda & Holster di paha kiri
    Vector3 lThighMid = VLerp(P.lHip, P.lKnee, 0.45f);
    DrawLitLimb(pos, yaw, VAdd(lThighMid, V3(0.0f,  0.022f, 0.0f)), VAdd(lThighMid, V3(0.0f,  0.002f, 0.0f)), 0.082f, 0.081f, leatherGear, leatherGear, 0.5f, 12);
    DrawLitLimb(pos, yaw, VAdd(lThighMid, V3(0.0f, -0.022f, 0.0f)), VAdd(lThighMid, V3(0.0f, -0.042f, 0.0f)), 0.079f, 0.078f, leatherGear, leatherGear, 0.5f, 12);
    DrawLitLimb(pos, yaw, VAdd(lThighMid, V3(-0.082f, 0.04f, 0.0f)), VAdd(lThighMid, V3(-0.082f, -0.09f, 0.015f)), 0.022f, 0.018f, leatherGear, leatherGear, 0.5f, 8);

    // 7. Gambar Wajah V-Jaw, Mata Almond, Kacamata Emas Rantai & Rambut Hitam Panjang
    DrawSculptedHeadAndHair(P, pos, yaw, blink);
}