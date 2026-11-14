#pragma once
#include "anim/pose.h"
#include <stdlib.h>

inline void InitAnim(AnimState& s) {
    s.time = 0.0f;
    s.state = 0;
    s.prevState = 0;
    s.transition = 1.0f;
    s.blinkTimer = 2.4f;
    s.blinkAmount = 0.0f;
    s.lHairPos = {0, 1.4f, 0};
    s.lHairVel = {0, 0, 0};
    s.rHairPos = {0, 1.4f, 0};
    s.rHairVel = {0, 0, 0};
    s.lChainPos = {0, 1.5f, 0};
    s.lChainVel = {0, 0, 0};
    s.rChainPos = {0, 1.5f, 0};
    s.rChainVel = {0, 0, 0};
    s.velocity = {0, 0, 0};
    s.moveSpeed = 0.0f;
    s.focusMode = false;
}

inline float BlinkAmount(const AnimState& s) {
    return s.blinkAmount;
}

inline void UpdateAnim(AnimState& s, float dt, Vector3 targetBase, float targetYaw) {
    s.time += dt;
    s.transition += dt * 10.0f;
    if (s.transition > 1.0f) s.transition = 1.0f;
    
    s.blinkTimer -= dt;
    if (s.blinkTimer <= 0.0f) {
        s.blinkAmount = 1.0f;
        s.blinkTimer = 2.4f + (rand() % 280) * 0.01f;
    } else if (s.blinkTimer > s.blinkTimer - 0.15f) {
        s.blinkAmount -= dt * 6.6f;
        if (s.blinkAmount < 0.0f) s.blinkAmount = 0.0f;
    }
    
    Basis3D b = MakeBasis(RotateYaw({0, 0, 1}, targetYaw));
    Vector3 lHTarget = VAdd(targetBase, BasisPoint(b, {-0.15f, 1.25f, 0.15f}));
    Vector3 rHTarget = VAdd(targetBase, BasisPoint(b, {0.15f, 1.25f, 0.15f}));
    Vector3 lCTarget = VAdd(targetBase, BasisPoint(b, {-0.09f, 1.40f, 0.12f}));
    Vector3 rCTarget = VAdd(targetBase, BasisPoint(b, {0.09f, 1.40f, 0.12f}));
    
    float k = 150.0f, damp = 0.85f;
    s.lHairVel = VMul(VAdd(s.lHairVel, VMul(VSub(lHTarget, s.lHairPos), k * dt)), damp);
    s.rHairVel = VMul(VAdd(s.rHairVel, VMul(VSub(rHTarget, s.rHairPos), k * dt)), damp);
    s.lChainVel = VMul(VAdd(s.lChainVel, VMul(VSub(lCTarget, s.lChainPos), k * dt)), damp);
    s.rChainVel = VMul(VAdd(s.rChainVel, VMul(VSub(rCTarget, s.rChainPos), k * dt)), damp);
    
    s.lHairPos = VAdd(s.lHairPos, VMul(s.lHairVel, dt));
    s.rHairPos = VAdd(s.rHairPos, VMul(s.rHairVel, dt));
    s.lChainPos = VAdd(s.lChainPos, VMul(s.lChainVel, dt));
    s.rChainPos = VAdd(s.rChainPos, VMul(s.rChainVel, dt));
}