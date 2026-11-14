#pragma once
#include "anim/pose.h"

inline void ApplySlidePose(FemalePose& p, const AnimState& s) {
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, p.yaw));
    p.root.y -= 0.6f;
    p.head.y -= 0.6f;
    p.lFoot = VAdd(p.root, BasisPoint(b, {-0.15f, 0.1f, 0.6f}));
    p.rFoot = VAdd(p.root, BasisPoint(b, {0.15f, 0.2f, -0.3f}));
    p.lHand = VAdd(p.root, BasisPoint(b, {-0.3f, 0.3f, -0.1f}));
    p.rHand = VAdd(p.root, BasisPoint(b, {0.3f, 0.5f, 0.4f}));
}