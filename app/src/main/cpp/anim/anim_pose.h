#pragma once
#include "anim/anim_run.h"
#include "anim/anim_slide.h"
#include "anim/anim_jump.h"

inline FemalePose ComputeFemalePose(const AnimState& s, Vector3 pos, float yaw) {
    FemalePose p;
    p.root = pos;
    p.yaw = yaw;
    p.head = VAdd(pos, {0, 1.52f, 0});
    p.breathOffset = sinf(s.time * 2.5f) * 0.015f;
    
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, yaw));
    
    FemalePose t = p;
    t.lFoot = VAdd(pos, BasisPoint(b, {-0.12f, 0, 0}));
    t.rFoot = VAdd(pos, BasisPoint(b, {0.12f, 0, 0}));
    t.lHand = VAdd(pos, BasisPoint(b, {-0.25f, 0.8f, 0}));
    t.rHand = VAdd(pos, BasisPoint(b, {0.25f, 0.8f, 0}));
    
    if (s.focusMode) {
        t.lHand = VAdd(pos, BasisPoint(b, {-0.15f, 1.2f, 0.4f}));
        t.rHand = VAdd(pos, BasisPoint(b, {0.15f, 1.15f, 0.45f}));
    }
    
    if (s.state == 1) {
        float phase = s.time * 4.0f;
        t.lFoot = VAdd(pos, BasisPoint(b, {-0.12f, fmaxf(0.0f, -cosf(phase)) * 0.15f, sinf(phase) * 0.25f}));
        t.rFoot = VAdd(pos, BasisPoint(b, {0.12f, fmaxf(0.0f, cosf(phase)) * 0.15f, -sinf(phase) * 0.25f}));
    } else if (s.state == 2) {
        ApplyRunPose(t, s);
    } else if (s.state == 3) {
        t.root.y -= 0.35f;
        t.head.y -= 0.35f;
        t.lFoot = VAdd(pos, BasisPoint(b, {-0.15f, 0, 0.1f}));
        t.rFoot = VAdd(pos, BasisPoint(b, {0.15f, 0, -0.2f}));
        t.lHand = VAdd(pos, BasisPoint(b, {-0.2f, 0.5f, 0.2f}));
    } else if (s.state == 4) {
        ApplySlidePose(t, s);
    } else if (s.state == 5) {
        ApplyJumpPose(t, s);
    }
    
    p.lFoot = t.lFoot; p.rFoot = t.rFoot;
    p.lHand = t.lHand; p.rHand = t.rHand;
    p.root = t.root; p.head = t.head;
    
    p.lKneePole = VAdd(pos, BasisPoint(b, {-0.12f, 0.5f, 1.0f}));
    p.rKneePole = VAdd(pos, BasisPoint(b, {0.12f, 0.5f, 1.0f}));
    p.lElbowPole = VAdd(pos, BasisPoint(b, {-0.4f, 1.0f, -0.5f}));
    p.rElbowPole = VAdd(pos, BasisPoint(b, {0.4f, 1.0f, -0.5f}));
    
    p.lHairPos = s.lHairPos; p.rHairPos = s.rHairPos;
    p.lChainPos = s.lChainPos; p.rChainPos = s.rChainPos;
    
    return p;
}