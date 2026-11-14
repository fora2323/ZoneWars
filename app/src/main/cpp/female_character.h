#pragma once

#include "character.h"

inline Body FemaleBody()
{
    return Body{ 0.185f, 0.165f, 0.125f, 0.155f, 0.148f, 0.44f, 0.43f };
}

inline Style FemaleStyle()
{
    Style s;
    s.skin = Color{ 246, 224, 212, 255 };        // Kulit putih cerah sesuai referensi
    s.skinShadow = Color{ 224, 196, 184, 255 };
    s.top = Color{ 138, 20, 34, 255 };           // Dress rajut merah marun (Ada Wong style)
    s.topDark = Color{ 96, 12, 24, 255 };        // Aksen lipatan & kerah marun gelap
    s.bottom = Color{ 34, 30, 36, 255 };         // Stocking hitam transparan gelap
    s.skirtC = Color{ 132, 18, 32, 255 };        // Rok mini rajut merah marun
    s.hair = Color{ 16, 16, 20, 255 };           // Rambut hitam legam
    s.hairHighlight = Color{ 42, 42, 52, 255 };
    s.shoe = Color{ 18, 18, 22, 255 };           // Boots kulit hitam tinggi
    s.lip = Color{ 178, 42, 52, 255 };           // Bibir glossy merah marun
    s.hand = Color{ 22, 22, 26, 255 };           // Sarung tangan kulit hitam
    s.harness = Color{ 24, 22, 26, 255 };        // Sabuk & harness kulit hitam
    s.hairLen = 0.44f;                           // Rambut panjang terurai
    s.skirt = true;
    s.boots = true;
    s.tacticalHarness = true;
    return s;
}

inline void DrawGlassesAndHairFront(const Pose& P, Vector3 pos, float yaw, float r, const Style& st)
{
    Color gold = Color{ 218, 182, 68, 255 };
    Color tieCol = Color{ 28, 26, 32, 255 };

    // 1. Kacamata Oval Bingkai Emas + Rantai Emas Mengayun (Sesuai Referensi Wajah)
    for (int k = -1; k <= 1; k += 2) {
        float sx = (float)k;
        float cx = 0.054f * sx;
        float cy = 0.012f;
        float hw = 0.040f;
        float hh = 0.021f;

        Vector3 tl = OnFace(P.head, r, cx - hw, cy + hh);
        Vector3 tr = OnFace(P.head, r, cx + hw, cy + hh);
        Vector3 br = OnFace(P.head, r, cx + hw, cy - hh);
        Vector3 bl = OnFace(P.head, r, cx - hw, cy - hh);

        Cap(pos, yaw, tl, tr, 0.0042f, gold);
        Cap(pos, yaw, tr, br, 0.0042f, gold);
        Cap(pos, yaw, br, bl, 0.0042f, gold);
        Cap(pos, yaw, bl, tl, 0.0042f, gold);

        // Gagang kacamata ke telinga
        Vector3 outer = sx < 0.0f ? tl : tr;
        Vector3 earTip = V3(P.head.x + sx * (r + 0.016f), P.head.y + cy, P.head.z + 0.02f);
        Cap(pos, yaw, outer, earTip, 0.0035f, gold);

        // Rantai kacamata emas menjuntai dengan fisika ayunan
        Vector3 prevChain = earTip;
        for (int i = 1; i <= 6; i++) {
            float t = (float)i / 6.0f;
            float curveZ = sinf(t * 3.14159f) * 0.03f;
            Vector3 cPt = V3(
                P.head.x + sx * (r + 0.022f - t * 0.025f) + P.chainOffset.x * t,
                P.head.y - 0.01f - t * 0.29f + P.chainOffset.y * t * 0.5f,
                P.head.z + 0.02f + curveZ + P.chainOffset.z * t
            );
            Sph(pos, yaw, cPt, 0.0052f, gold);
            prevChain = cPt;
        }

        // 2. Dua Untai Rambut Depan dengan Ikatan (Sesuai Referensi Gambar 2)
        Vector3 fRoot = V3(P.head.x + sx * (r * 0.78f), P.head.y + 0.06f, P.head.z + 0.06f);
        Vector3 fCheek = V3(P.head.x + sx * (r * 0.92f), P.head.y - 0.06f, P.head.z + 0.08f);
        Vector3 fTie = V3(P.chest.x + sx * 0.105f + P.hairOffset.x * 0.35f,
                          P.chest.y + 0.03f + P.hairOffset.y * 0.25f,
                          P.chest.z + 0.135f + P.hairOffset.z * 0.25f);
        Vector3 fTip = V3(fTie.x + sx * 0.015f + P.hairOffset.x * 0.7f,
                          fTie.y - 0.21f + P.hairOffset.y * 0.5f,
                          fTie.z + 0.015f + P.hairOffset.z * 0.5f);

        Cap(pos, yaw, fRoot, fCheek, 0.026f, st.hair);
        Cap(pos, yaw, fCheek, fTie, 0.022f, st.hair);
        Sph(pos, yaw, fTie, 0.026f, tieCol); // Ikatan rambut hitam di dada atas
        Cap(pos, yaw, fTie, fTip, 0.020f, st.hair);
    }

    // Jembatan hidung kacamata emas
    Cap(pos, yaw, OnFace(P.head, r, -0.014f, 0.018f), OnFace(P.head, r, 0.014f, 0.018f), 0.0038f, gold);
}

inline void DrawTacticalGear(const Pose& P, Vector3 pos, float yaw, const Style& st)
{
    Color buckle = Color{ 190, 195, 205, 255 };

    // 1. Harness Bahu & Strap Dada Hitam (Sesuai Referensi Gambar 1)
    Vector3 chestL = VAdd(P.chest, V3(-0.10f, 0.06f, 0.13f));
    Vector3 chestR = VAdd(P.chest, V3(0.10f, 0.06f, 0.13f));
    Cap(pos, yaw, chestL, chestR, 0.015f, st.harness);
    Cap(pos, yaw, VAdd(P.lShoulder, V3(0.05f, 0.01f, 0.0f)), chestL, 0.018f, st.harness);
    Cap(pos, yaw, VAdd(P.rShoulder, V3(-0.05f, 0.01f, 0.0f)), chestR, 0.018f, st.harness);

    // Holster bawah lengan kiri
    Vector3 underArm = VAdd(P.chest, V3(-0.155f, -0.04f, 0.02f));
    Cap(pos, yaw, underArm, VAdd(underArm, V3(0.0f, -0.09f, 0.03f)), 0.026f, st.harness);

    // 2. Sabuk Pinggang Miring & Gesper Perak
    Vector3 beltL = VAdd(P.waist, V3(-0.15f, -0.03f, 0.11f));
    Vector3 beltR = VAdd(P.waist, V3(0.15f, -0.06f, 0.11f));
    Cap(pos, yaw, beltL, beltR, 0.016f, st.harness);
    Sph(pos, yaw, VAdd(P.waist, V3(0.04f, -0.045f, 0.13f)), 0.020f, buckle);

    // 3. Strap & Holster Paha Kiri
    Vector3 thighTop = V3(Lerpf(P.lHipJ.x, P.lKnee.x, 0.42f),
                          Lerpf(P.lHipJ.y, P.lKnee.y, 0.42f),
                          Lerpf(P.lHipJ.z, P.lKnee.z, 0.42f));
    Cap(pos, yaw, VAdd(thighTop, V3(-0.06f, 0.02f, 0.0f)), VAdd(thighTop, V3(0.06f, 0.02f, 0.0f)), 0.045f, st.harness);
    Cap(pos, yaw, VAdd(thighTop, V3(-0.085f, 0.04f, 0.0f)), VAdd(thighTop, V3(-0.085f, -0.08f, 0.02f)), 0.024f, st.harness);
}

inline void DrawFemaleCharacter(const Pose& P, Vector3 pos, float yaw, float blink)
{
    Body b = FemaleBody();
    Style st = FemaleStyle();

    // 1. Gambar Bayangan Real-Time di Lantai
    DrawCharacterShadow(P, pos, yaw);

    // 2. Gambar Tubuh, Dress Marun, Stocking, & Boots
    DrawHumanoid(P, pos, yaw, st, b, blink);

    // 3. Gambar Harness Taktis, Sabuk, & Holster
    DrawTacticalGear(P, pos, yaw, st);

    // 4. Gambar Kacamata Emas Rantai & Rambut Depan Ikat
    DrawGlassesAndHairFront(P, pos, yaw, b.headR, st);
}