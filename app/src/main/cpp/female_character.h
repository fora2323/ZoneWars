#pragma once

#include "render_helper.h"

// =========================================================
// 1. WAJAH PORSELEN CERAH, MATA ALMOND, KACAMATA EMAS & RAMBUT
// (Rambut bagian belakang diposisikan di Z negatif sehingga TIDAK PERNAH
//  menutupi kulit wajah dari depan!)
// =========================================================
inline void DrawSculptedHeadAndHair(const FemalePose& P, Vector3 rootPos, float rootYaw, float blink)
{
    const Color skinBase   = Color{ 252, 236, 226, 255 }; // Kulit wajah porselen cerah
    const Color skinBlush  = Color{ 248, 212, 208, 255 }; // Pipi merona halus
    const Color lipUpper   = Color{ 185, 45, 58, 255 };   // Bibir merah rose
    const Color lipLower   = Color{ 210, 62, 74, 255 };   // Bibir bawah glossy
    const Color eyeWhite   = Color{ 250, 250, 255, 255 };
    const Color irisCol    = Color{ 115, 78, 62, 255 };   // Iris cokelat hazel
    const Color lashCol    = Color{ 20, 16, 20, 255 };    // Eyeliner & bulu mata hitam
    const Color hairBlack  = Color{ 20, 20, 26, 255 };    // Rambut hitam berkilau
    const Color hairTieCol = Color{ 45, 12, 18, 255 };    // Ikatan rambut depan
    const Color goldCol    = Color{ 238, 202, 90, 255 };  // Bingkai & rantai kacamata emas

    Vector3 worldHeadPos = WorldPoint(rootPos, P.headCenter, rootYaw);
    Basis3D H = MakeBasis(worldHeadPos, rootYaw + P.headYawOff, P.headPitch, P.headRoll);

    // A. KONTUR WAJAH V-SHAPE (Dagu lancip, pipi mulus, dahi tertutup poni atas)
    LoftSlice headSlices[7] = {
        { V3(0.0f, -0.108f,  0.024f), 0.018f, 0.022f, skinBase,  0.12f }, // Ujung dagu V-shape
        { V3(0.0f, -0.075f,  0.012f), 0.064f, 0.074f, skinBase,  0.12f }, // Rahang bawah tirus
        { V3(0.0f, -0.030f,  0.006f), 0.092f, 0.104f, skinBlush, 0.15f }, // Pipi
        { V3(0.0f,  0.015f,  0.002f), 0.100f, 0.112f, skinBase,  0.15f }, // Area mata & tulang pipi
        { V3(0.0f,  0.062f, -0.002f), 0.098f, 0.110f, skinBase,  0.12f }, // Dahi
        { V3(0.0f,  0.102f, -0.008f), 0.082f, 0.096f, hairBlack, 0.45f }, // Batas rambut atas
        { V3(0.0f,  0.126f, -0.015f), 0.032f, 0.040f, hairBlack, 0.45f }  // Puncak kepala
    };
    DrawBasisLoft(H, headSlices, 7, 16);

    // B. HIDUNG MANCUNG & BIBIR MERAH GLOSSY (Di permukaan depan Z = +0.112..+0.126)
    DrawBasisTube(H, V3(0.0f, 0.016f, 0.112f), V3(0.0f, -0.024f, 0.126f), 0.008f, skinBase, 0.25f);
    DrawLitEllipsoid(H, V3(0.0f, -0.025f, 0.124f), V3(0.010f, 0.008f, 0.010f), skinBase, 0.25f, 5, 8);

    // Bibir atas & bawah
    DrawBasisTube(H, V3(-0.020f, -0.055f, 0.104f), V3(0.0f, -0.052f, 0.112f), 0.006f, lipUpper, 0.5f);
    DrawBasisTube(H, V3( 0.000f, -0.052f, 0.112f), V3(0.020f, -0.055f, 0.104f), 0.006f, lipUpper, 0.5f);
    DrawLitEllipsoid(H, V3(0.0f, -0.062f, 0.108f), V3(0.019f, 0.007f, 0.009f), lipLower, 0.7f, 5, 8);

    // C. MATA ALMOND PROPORSIONAL + EYELINER + ALIS + KACAMATA EMAS RANTAI
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        float ex = 0.042f * sx;
        float ey = 0.014f;
        float ez = 0.104f;

        if (blink < 0.85f) {
            float openScale = 1.0f - blink * 0.9f;
            // Putih mata (Sclera)
            DrawLitEllipsoid(H, V3(ex, ey, ez), V3(0.018f, 0.0095f * openScale, 0.006f), eyeWhite, 0.3f, 5, 8);
            // Iris cokelat & pupil hitam di depannya
            DrawLitEllipsoid(H, V3(ex, ey, ez + 0.004f), V3(0.0105f, 0.009f * openScale, 0.004f), irisCol, 0.4f, 5, 8);
            DrawLitEllipsoid(H, V3(ex, ey, ez + 0.006f), V3(0.0055f, 0.0055f * openScale, 0.003f), lashCol, 0.5f, 4, 6);
            // Pantulan cahaya mata (Catchlight kecil)
            DrawLitEllipsoid(H, V3(ex - 0.0035f * sx, ey + 0.003f * openScale, ez + 0.0085f),
                             V3(0.0025f, 0.0025f, 0.002f), eyeWhite, 0.9f, 4, 6);
        }

        // Garis Eyeliner Winged
        float lidDrop = blink * 0.010f;
        Vector3 inC  = V3(ex - 0.017f * sx, ey - 0.002f, ez + 0.003f);
        Vector3 midC = V3(ex + 0.002f * sx, ey + 0.009f - lidDrop, ez + 0.006f);
        Vector3 outC = V3(ex + 0.023f * sx, ey + 0.004f - lidDrop * 0.5f, ez - 0.001f);
        DrawBasisTube(H, inC, midC, 0.0026f, lashCol, 0.3f);
        DrawBasisTube(H, midC, outC, 0.0028f, lashCol, 0.3f);

        // Alis rapi
        DrawBasisTube(H, V3(ex - 0.015f * sx, ey + 0.025f, ez + 0.004f),
                         V3(ex + 0.006f * sx, ey + 0.030f, ez + 0.002f), 0.0032f, lashCol, 0.3f);
        DrawBasisTube(H, V3(ex + 0.006f * sx, ey + 0.030f, ez + 0.002f),
                         V3(ex + 0.026f * sx, ey + 0.022f, ez - 0.004f), 0.0025f, lashCol, 0.3f);

        // Bingkai Kacamata Oval Emas (di depan mata Z = +0.120)
        float gx = 0.044f * sx;
        float gy = 0.009f;
        float gz = 0.120f;
        const int gSegs = 10;
        const float twoPi = 6.2831853f;
        for (int i = 0; i < gSegs; i++) {
            float a0 = (float)i / (float)gSegs * twoPi;
            float a1 = (float)(i + 1) / (float)gSegs * twoPi;
            Vector3 p0 = V3(gx + cosf(a0) * 0.030f, gy + sinf(a0) * 0.017f, gz - fabsf(cosf(a0)) * 0.004f);
            Vector3 p1 = V3(gx + cosf(a1) * 0.030f, gy + sinf(a1) * 0.017f, gz - fabsf(cosf(a1)) * 0.004f);
            DrawBasisTube(H, p0, p1, 0.0022f, goldCol, 0.85f);
        }

        // Gagang & Rantai Kacamata Emas Menjuntai di Samping Pipi
        Vector3 hinge = V3(gx + 0.031f * sx, gy + 0.003f, gz - 0.006f);
        Vector3 earPt = V3(0.098f * sx, gy, 0.010f);
        DrawBasisTube(H, hinge, earPt, 0.0020f, goldCol, 0.85f);

        Vector3 prevLink = hinge;
        for (int c = 1; c <= 6; c++) {
            float t = (float)c / 6.0f;
            Vector3 nextLink = V3(
                hinge.x + sx * 0.010f + P.chainFlow.x * t * 0.5f,
                hinge.y - t * 0.25f + P.chainFlow.y * t * 0.3f,
                hinge.z - t * 0.03f + P.chainFlow.z * t * 0.5f
            );
            DrawBasisTube(H, prevLink, nextLink, 0.0018f, goldCol, 0.9f);
            prevLink = nextLink;
        }
    }
    // Jembatan hidung kacamata emas
    DrawBasisTube(H, V3(-0.014f, 0.012f, 0.121f), V3(0.014f, 0.012f, 0.121f), 0.0022f, goldCol, 0.85f);

    // D. RAMBUT BELAKANG, PONI SAMPING, & 2 KUNCIR DEPAN
    // Tempurung rambut belakang (Digeser ke Z = -0.042f agar TIDAK PERNAH menembus wajah depan!)
    DrawLitEllipsoid(H, V3(0.0f, 0.032f, -0.042f), V3(0.108f, 0.112f, 0.095f), hairBlack, 0.5f, 7, 12);

    // Poni belah samping membingkai pelipis & pipi luar (Tidak menutupi mata!)
    for (int side = -1; side <= 1; side += 2) {
        float sx = (float)side;
        DrawBasisTube(H, V3(0.012f * sx, 0.112f, 0.082f), V3(0.072f * sx, 0.062f, 0.092f), 0.022f, hairBlack, 0.55f);
        DrawBasisTube(H, V3(0.072f * sx, 0.062f, 0.092f), V3(0.098f * sx, -0.015f, 0.055f), 0.020f, hairBlack, 0.55f);

        // Dua Untai Rambut Depan Terikat di Atas Dada (Sesuai Referensi Gambar 2)
        Vector3 fTop = VAdd(P.headCenter, V3(0.092f * sx, -0.03f, 0.035f));
        Vector3 fTie = VAdd(P.chest, V3(0.088f * sx, 0.03f, 0.122f));
        Vector3 fTip = VAdd(P.chest, V3(0.095f * sx + P.hairFlow.x * 0.3f, -0.17f, 0.115f));
        DrawLitLimb(rootPos, rootYaw, fTop, fTie, 0.021f, 0.018f, hairBlack, hairBlack, 0.5f, 8);
        DrawLitLimb(rootPos, rootYaw, VAdd(fTie, V3(0.0f, 0.014f, 0.0f)), VAdd(fTie, V3(0.0f, -0.014f, 0.0f)),
                    0.023f, 0.023f, hairTieCol, hairTieCol, 0.3f, 8);
        DrawLitLimb(rootPos, rootYaw, fTie, fTip, 0.018f, 0.010f, hairBlack, hairBlack, 0.5f, 8);
    }

    // Rambut Panjang Belakang (Selalu berada di belakang punggung Z <= -0.11f agar tidak masuk ke baju)
    float backZ = fminf(-0.11f, -0.11f + P.hairFlow.z * 0.6f);
    LoftSlice backHair[5] = {
        { VAdd(P.headCenter, V3(0.0f,  0.03f, -0.075f)),                 0.105f, 0.045f, hairBlack, 0.5f },
        { VAdd(P.neckBase,   V3(P.hairFlow.x * 0.2f,  0.01f, -0.095f)),  0.122f, 0.042f, hairBlack, 0.5f },
        { VAdd(P.chest,      V3(P.hairFlow.x * 0.4f,  0.01f,  backZ)),   0.135f, 0.038f, hairBlack, 0.5f },
        { VAdd(P.waist,      V3(P.hairFlow.x * 0.7f,  0.04f,  backZ - 0.02f)), 0.125f, 0.032f, hairBlack, 0.5f },
        { VAdd(P.waist,      V3(P.hairFlow.x * 1.0f, -0.12f,  backZ - 0.03f)), 0.080f, 0.022f, hairBlack, 0.5f }
    };
    DrawLoftedMesh(rootPos, rootYaw, backHair, 5, 10);
}

// =========================================================
// 2. TUBUH HOURGLASS, SWEATER DRESS MERAH MARUN, HARNESS & BOOTS
// =========================================================
inline void DrawFemaleCharacter(const FemalePose& P, Vector3 pos, float yaw, float blink)
{
    const Color skinCol      = Color{ 252, 236, 226, 255 };
    const Color dressBright  = Color{ 156, 24, 40, 255 };
    const Color dressMid     = Color{ 128, 18, 32, 255 };
    const Color dressDeep    = Color{ 94, 12, 24, 255 };
    const Color stockingCol  = Color{ 34, 30, 36, 255 };
    const Color stockingTop  = Color{ 24, 22, 26, 255 };
    const Color bootLeather  = Color{ 18, 18, 22, 255 };
    const Color leatherGear  = Color{ 24, 22, 26, 255 };
    const Color silverBuckle = Color{ 210, 215, 225, 255 };

    // 1. Gambar Bayangan Datar Bebas Kedip
    DrawFemaleShadow(P, pos, yaw);

    // 2. TUBUH HOURGLASS & DRESS MERAH MARUN
    Vector3 hipMid = P.pelvis;
    Vector3 wst    = P.waist;
    Vector3 chs    = P.chest;
    Vector3 nck    = P.neckBase;

    // Rok bawah mengikuti arah rata-rata paha agar paha tidak menembus rok saat jongkok/slide
    float legAvgZ = (P.lKnee.z + P.rKnee.z) * 0.24f;
    float legAvgY = (P.lKnee.y + P.rKnee.y) * 0.12f;

    LoftSlice dressSlices[9] = {
        { V3(hipMid.x, hipMid.y - 0.20f + legAvgY, hipMid.z + legAvgZ), 0.175f, 0.125f, dressDeep,   0.18f },
        { V3(hipMid.x, hipMid.y - 0.08f + legAvgY * 0.5f, hipMid.z - 0.010f + legAvgZ * 0.5f), 0.185f, 0.136f, dressBright, 0.22f },
        { V3(hipMid.x, hipMid.y + 0.02f, hipMid.z - 0.005f),            0.172f, 0.120f, dressMid,    0.20f },
        { V3(wst.x,    wst.y,            wst.z),                        0.115f, 0.086f, dressDeep,   0.15f },
        { V3(chs.x,    chs.y - 0.08f,    chs.z),                        0.135f, 0.095f, dressMid,    0.20f },
        { V3(chs.x,    chs.y + 0.01f,    chs.z + 0.015f),               0.152f, 0.114f, dressBright, 0.25f },
        { V3(chs.x,    chs.y + 0.085f,   chs.z),                        0.162f, 0.088f, dressMid,    0.20f },
        { V3(nck.x,    nck.y - 0.01f,    nck.z),                        0.060f, 0.056f, dressDeep,   0.18f },
        { V3(nck.x,    nck.y + 0.05f,    nck.z),                        0.054f, 0.052f, dressBright, 0.22f }
    };
    DrawLoftedMesh(pos, yaw, dressSlices, 9, 16);

    // Leher atas menyambung ke dagu
    DrawLitLimb(pos, yaw, VAdd(nck, V3(0.0f, 0.04f, 0.0f)), P.headCenter, 0.046f, 0.042f, skinCol, skinCol, 0.15f, 10);

    // Kontur Dada
    Basis3D chestBasis = MakeBasis(WorldPoint(pos, chs, yaw), yaw + P.spineTwist * 0.5f, P.torsoLean, 0.0f);
    for (int side = -1; side <= 1; side += 2) {
        DrawLitEllipsoid(chestBasis, V3(0.064f * (float)side, 0.005f + P.bustBounce, 0.062f),
                         V3(0.068f, 0.062f, 0.065f), dressBright, 0.25f, 6, 10);
    }

    // 3. LENGAN & SARUNG TANGAN KULIT HITAM
    DrawLitLimb(pos, yaw, P.lShoulder, P.lElbow, 0.050f, 0.040f, dressBright, dressMid, 0.20f, 10);
    DrawLitLimb(pos, yaw, P.lElbow, P.lWrist,    0.039f, 0.032f, dressMid, dressDeep,   0.20f, 10);
    DrawLitLimb(pos, yaw, P.lWrist, P.lHand,     0.035f, 0.026f, leatherGear, leatherGear, 0.50f, 8);

    DrawLitLimb(pos, yaw, P.rShoulder, P.rElbow, 0.050f, 0.040f, dressBright, dressMid, 0.20f, 10);
    DrawLitLimb(pos, yaw, P.rElbow, P.rWrist,    0.039f, 0.032f, dressMid, dressDeep,   0.20f, 10);
    DrawLitLimb(pos, yaw, P.rWrist, P.rHand,     0.035f, 0.026f, leatherGear, leatherGear, 0.50f, 8);

    // 4. KAKI (STOCKING & BOOTS TINGGI)
    DrawLitLimb(pos, yaw, P.lHip, P.lKnee, 0.086f, 0.058f, stockingTop, stockingCol, 0.30f, 12);
    DrawLitLimb(pos, yaw, P.rHip, P.rKnee, 0.086f, 0.058f, stockingTop, stockingCol, 0.30f, 12);

    Vector3 lBootCuff = VLerp(P.lHip, P.lKnee, 0.90f);
    Vector3 rBootCuff = VLerp(P.rHip, P.rKnee, 0.90f);
    DrawLitLimb(pos, yaw, lBootCuff, P.lKnee,  0.064f, 0.062f, bootLeather, bootLeather, 0.60f, 12);
    DrawLitLimb(pos, yaw, P.lKnee,   P.lAnkle, 0.060f, 0.044f, bootLeather, bootLeather, 0.60f, 12);
    DrawLitLimb(pos, yaw, rBootCuff, P.rKnee,  0.064f, 0.062f, bootLeather, bootLeather, 0.60f, 12);
    DrawLitLimb(pos, yaw, P.rKnee,   P.rAnkle, 0.060f, 0.044f, bootLeather, bootLeather, 0.60f, 12);

    DrawLitLimb(pos, yaw, P.lAnkle, P.lToe, 0.044f, 0.026f, bootLeather, bootLeather, 0.65f, 8);
    DrawLitLimb(pos, yaw, P.rAnkle, P.rToe, 0.044f, 0.026f, bootLeather, bootLeather, 0.65f, 8);

    // Hak Sepatu Boots
    DrawLitLimb(pos, yaw, VAdd(P.lAnkle, V3(0.0f, -0.01f, -0.025f)), VAdd(P.lAnkle, V3(0.0f, -0.08f, -0.025f)), 0.015f, 0.009f, bootLeather, bootLeather, 0.65f, 6);
    DrawLitLimb(pos, yaw, VAdd(P.rAnkle, V3(0.0f, -0.01f, -0.025f)), VAdd(P.rAnkle, V3(0.0f, -0.08f, -0.025f)), 0.015f, 0.009f, bootLeather, bootLeather, 0.65f, 6);

    // 5. HARNESS BAHU, SABUK PINGGANG, & HOLSTER PAHA
    DrawBasisTube(chestBasis, V3(-0.09f, 0.06f, 0.095f), V3(0.09f, 0.06f, 0.095f), 0.012f, leatherGear, 0.5f);
    DrawBasisTube(chestBasis, V3(-0.12f, 0.08f, 0.000f), V3(-0.09f, 0.06f, 0.095f), 0.013f, leatherGear, 0.5f);
    DrawBasisTube(chestBasis, V3( 0.12f, 0.08f, 0.000f), V3( 0.09f, 0.06f, 0.095f), 0.013f, leatherGear, 0.5f);

    Basis3D pelvisBasis = MakeBasis(WorldPoint(pos, hipMid, yaw), yaw, 0.0f, 0.0f);
    DrawBasisTube(pelvisBasis, V3(-0.15f, 0.10f, 0.095f), V3(0.15f, 0.06f, 0.095f), 0.013f, leatherGear, 0.5f);
    DrawLitEllipsoid(pelvisBasis, V3(0.03f, 0.08f, 0.105f), V3(0.016f, 0.012f, 0.008f), silverBuckle, 0.8f, 4, 6);

    Vector3 lThighMid = VLerp(P.lHip, P.lKnee, 0.45f);
    DrawLitLimb(pos, yaw, VAdd(lThighMid, V3(-0.078f, 0.04f, 0.0f)), VAdd(lThighMid, V3(-0.078f, -0.08f, 0.01f)), 0.020f, 0.016f, leatherGear, leatherGear, 0.5f, 6);

    // 6. Gambar Kepala, Wajah, Kacamata Emas, & Rambut
    DrawSculptedHeadAndHair(P, pos, yaw, blink);
}