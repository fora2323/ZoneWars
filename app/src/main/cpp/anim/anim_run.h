#pragma once
#include "anim/pose.h"

inline void ApplyRunPose(FemalePose& p, const AnimState& s) {
    float t = s.time * 2.0f;
    float cycle = t - floorf(t);
    float lPhase = cycle * M_PI * 2.0f;
    float rPhase = lPhase + M_PI;
    
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, p.yaw));
    p.lFoot = VAdd(p.root, BasisPoint(b, {-0.12f, fmaxf(0.0f, -cosf(lPhase)) * 0.3f, sinf(lPhase) * 0.4f}));
    p.rFoot = VAdd(p.root, BasisPoint(b, {0.12f, fmaxf(0.0f, -cosf(rPhase)) * 0.3f, sinf(rPhase) * 0.4f}));
    p.lHand = VAdd(p.root, BasisPoint(b, {-0.25f, 0.9f + sinf(lPhase) * 0.15f, -sinf(lPhase) * 0.3f}));
    p.rHand = VAdd(p.root, BasisPoint(b, {0.25f, 0.9f + sinf(rPhase) * 0.15f, -sinf(rPhase) * 0.3f}));
}