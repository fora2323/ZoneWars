#pragma once
#include "render/primitives.h"
#include "anim/pose.h"

inline void DrawFemaleBody(const FemalePose& p) {
    Color colSkin = {255, 224, 208, 255};
    Color colDress1 = {156, 24, 40, 255};
    Color colDress2 = {128, 15, 32, 255};
    Color colDress3 = {94, 12, 24, 255};
    Color colLeather = {30, 30, 30, 255};
    Color colGlove = {20, 20, 20, 255};
    
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, p.yaw));
    
    float bo = p.breathOffset;
    LoftSlice dressSlices[] = {
        {VAdd(p.root, BasisPoint(b, {0, 0.70f, 0})), 0.20f, colDress3},
        {VAdd(p.root, BasisPoint(b, {0, 0.82f, 0})), 0.23f, colDress2},
        {VAdd(p.root, BasisPoint(b, {0, 0.94f, 0})), 0.25f, colDress1},
        {VAdd(p.root, BasisPoint(b, {0, 1.05f, 0})), 0.16f, colDress1},
        {VAdd(p.root, BasisPoint(b, {0, 1.15f, 0})), 0.19f, colDress2},
        {VAdd(p.root, BasisPoint(b, {0, 1.25f + bo, 0.02f})), 0.21f, colDress1},
        {VAdd(p.root, BasisPoint(b, {0, 1.35f + bo, 0.01f})), 0.22f, colDress2},
        {VAdd(p.root, BasisPoint(b, {0, 1.42f, 0})), 0.18f, colDress3},
        {VAdd(p.root, BasisPoint(b, {0, 1.46f, 0})), 0.08f, colDress3},
        {VAdd(p.root, BasisPoint(b, {0, 1.50f, 0})), 0.08f, colDress1}
    };
    DrawSmoothLoft(dressSlices, 10, 4, b);
    
    DrawLitEllipsoid(VAdd(p.root, BasisPoint(b, {-0.09f, 1.28f + bo, 0.18f})), {0.11f, 0.11f, 0.14f}, b, colDress1);
    DrawLitEllipsoid(VAdd(p.root, BasisPoint(b, {0.09f, 1.28f + bo, 0.18f})), {0.11f, 0.11f, 0.14f}, b, colDress1);
    DrawLitEllipsoid(VAdd(p.root, BasisPoint(b, {-0.10f, 0.92f, -0.12f})), {0.14f, 0.15f, 0.16f}, b, colDress2);
    DrawLitEllipsoid(VAdd(p.root, BasisPoint(b, {0.10f, 0.92f, -0.12f})), {0.14f, 0.15f, 0.16f}, b, colDress2);
    
    Vector3 lShoulder = VAdd(p.root, BasisPoint(b, {-0.18f, 1.38f, 0}));
    Vector3 rShoulder = VAdd(p.root, BasisPoint(b, {0.18f, 1.38f, 0}));
    Vector3 lElbow = TwoBoneIK(lShoulder, p.lHand, p.lElbowPole, 0.28f, 0.26f);
    Vector3 rElbow = TwoBoneIK(rShoulder, p.rHand, p.rElbowPole, 0.28f, 0.26f);
    
    DrawLitLimb(lShoulder, lElbow, 0.06f, 0.05f, colDress2, colDress3);
    DrawLitLimb(lElbow, p.lHand, 0.05f, 0.04f, colDress3, colDress2);
    DrawLitLimb(rShoulder, rElbow, 0.06f, 0.05f, colDress2, colDress3);
    DrawLitLimb(rElbow, p.rHand, 0.05f, 0.04f, colDress3, colDress2);
    DrawLitEllipsoid(p.lHand, {0.04f, 0.06f, 0.03f}, b, colGlove);
    DrawLitEllipsoid(p.rHand, {0.04f, 0.06f, 0.03f}, b, colGlove);
    
    Vector3 lHip = VAdd(p.root, BasisPoint(b, {-0.12f, 0.85f, 0}));
    Vector3 rHip = VAdd(p.root, BasisPoint(b, {0.12f, 0.85f, 0}));
    Vector3 lKnee = TwoBoneIK(lHip, p.lFoot, p.lKneePole, 0.42f, 0.40f);
    Vector3 rKnee = TwoBoneIK(rHip, p.rFoot, p.rKneePole, 0.42f, 0.40f);
    
    DrawLitLimb(lHip, lKnee, 0.11f, 0.08f, colSkin, colSkin);
    DrawLitLimb(rHip, rKnee, 0.11f, 0.08f, colSkin, colSkin);
    
    Vector3 lMidCalf = VLerp(lKnee, p.lFoot, 0.2f);
    Vector3 rMidCalf = VLerp(rKnee, p.rFoot, 0.2f);
    DrawLitLimb(lMidCalf, p.lFoot, 0.07f, 0.05f, colLeather, colLeather);
    DrawLitLimb(rMidCalf, p.rFoot, 0.07f, 0.05f, colLeather, colLeather);
    DrawLitEllipsoid(VAdd(p.lFoot, BasisPoint(b, {0, 0, 0.05f})), {0.05f, 0.04f, 0.10f}, b, colLeather);
    DrawLitEllipsoid(VAdd(p.rFoot, BasisPoint(b, {0, 0, 0.05f})), {0.05f, 0.04f, 0.10f}, b, colLeather);
    
    Vector3 hFront = VAdd(p.root, BasisPoint(b, {0, 1.34f + bo, 0.22f}));
    Vector3 hBack = VAdd(p.root, BasisPoint(b, {0, 1.34f, -0.19f}));
    DrawBasisTube(hFront, lShoulder, 0.015f, 0.015f, colLeather, colLeather);
    DrawBasisTube(hFront, rShoulder, 0.015f, 0.015f, colLeather, colLeather);
    DrawBasisTube(hBack, lShoulder, 0.015f, 0.015f, colLeather, colLeather);
    DrawBasisTube(hBack, rShoulder, 0.015f, 0.015f, colLeather, colLeather);
    
    Vector3 beltL = VAdd(p.root, BasisPoint(b, {-0.17f, 1.05f, 0}));
    Vector3 beltR = VAdd(p.root, BasisPoint(b, {0.17f, 1.03f, 0}));
    DrawBasisTube(beltL, beltR, 0.02f, 0.02f, colLeather, colLeather);
    
    Vector3 rThigh = VLerp(rHip, rKnee, 0.3f);
    DrawLitEllipsoid(VAdd(rThigh, BasisPoint(b, {0.12f, 0, 0})), {0.04f, 0.10f, 0.08f}, b, colLeather);
    DrawLitEllipsoid(VAdd(rThigh, BasisPoint(b, {0.14f, -0.05f, 0.05f})), {0.02f, 0.03f, 0.06f}, b, {50, 50, 50, 255});
}