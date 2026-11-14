#pragma once
#include "render/primitives.h"
#include "anim/pose.h"

inline void DrawFemaleHead(const FemalePose& p, float blink) {
    Color colSkin = {255, 230, 215, 255};
    Color colHair = {15, 15, 18, 255};
    Color colLip = {180, 40, 50, 255};
    Color colGold = {212, 175, 55, 255};
    Color colEyeW = {240, 240, 240, 255};
    Color colEyeB = {20, 15, 10, 255};
    Color colTie = {94, 12, 24, 255};
    
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, p.yaw));
    Vector3 hc = p.head;
    
    DrawLitEllipsoid(VAdd(hc, BasisPoint(b, {0, 0.08f, 0.02f})), {0.08f, 0.11f, 0.09f}, b, colSkin);
    
    float eyeOpen = 1.0f - blink;
    Vector3 lEye = VAdd(hc, BasisPoint(b, {-0.035f, 0.08f, 0.10f}));
    Vector3 rEye = VAdd(hc, BasisPoint(b, {0.035f, 0.08f, 0.10f}));
    DrawLitEllipsoid(lEye, {0.015f, 0.008f * eyeOpen, 0.01f}, b, colEyeW);
    DrawLitEllipsoid(rEye, {0.015f, 0.008f * eyeOpen, 0.01f}, b, colEyeW);
    DrawLitEllipsoid(VAdd(lEye, BasisPoint(b, {0, 0, 0.008f})), {0.006f, 0.006f * eyeOpen, 0.005f}, b, colEyeB);
    DrawLitEllipsoid(VAdd(rEye, BasisPoint(b, {0, 0, 0.008f})), {0.006f, 0.006f * eyeOpen, 0.005f}, b, colEyeB);
    
    DrawLitEllipsoid(VAdd(hc, BasisPoint(b, {0, 0.02f, 0.105f})), {0.02f, 0.01f, 0.015f}, b, colLip);
    
    Vector3 lEar = VAdd(hc, BasisPoint(b, {-0.085f, 0.08f, 0.01f}));
    Vector3 rEar = VAdd(hc, BasisPoint(b, {0.085f, 0.08f, 0.01f}));
    DrawBasisTube(lEar, lEye, 0.002f, 0.002f, colGold, colGold);
    DrawBasisTube(rEar, rEye, 0.002f, 0.002f, colGold, colGold);
    DrawBasisTube(lEye, rEye, 0.002f, 0.002f, colGold, colGold);
    DrawBasisTube(lEar, p.lChainPos, 0.001f, 0.001f, colGold, colGold);
    DrawBasisTube(rEar, p.rChainPos, 0.001f, 0.001f, colGold, colGold);
    
    DrawLitEllipsoid(VAdd(hc, BasisPoint(b, {0, 0.12f, -0.02f})), {0.09f, 0.10f, 0.11f}, b, colHair);
    
    LoftSlice lTail[] = {
        {VAdd(hc, BasisPoint(b, {-0.08f, 0.0f, 0.02f})), 0.03f, colHair},
        {VAdd(p.lHairPos, BasisPoint(b, {0, 0.05f, 0})), 0.015f, colTie},
        {p.lHairPos, 0.02f, colHair}
    };
    LoftSlice rTail[] = {
        {VAdd(hc, BasisPoint(b, {0.08f, 0.0f, 0.02f})), 0.03f, colHair},
        {VAdd(p.rHairPos, BasisPoint(b, {0, 0.05f, 0})), 0.015f, colTie},
        {p.rHairPos, 0.02f, colHair}
    };
    DrawSmoothLoft(lTail, 3, 2, b);
    DrawSmoothLoft(rTail, 3, 2, b);
}