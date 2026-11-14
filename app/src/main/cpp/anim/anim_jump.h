#pragma once
#include "anim/pose.h"

inline void ApplyJumpPose(FemalePose& p, const AnimState& s) {
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, p.yaw));
    float h = fmaxf(0.0f, p.root.y - 0.1f);
    p.lFoot = VAdd(p.root, BasisPoint(b, {-0.15f, 0.3f + h * 0.1f, -0.2f}));
    p.rFoot = VAdd(p.root, BasisPoint(b, {0.15f, 0.1f + h * 0.1f, 0.1f}));
    p.lHand = VAdd(p.root, BasisPoint(b, {-0.3f, 1.3f, 0.2f}));
    p.rHand = VAdd(p.root, BasisPoint(b, {0.3f, 1.4f, 0.1f}));
}