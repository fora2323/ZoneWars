#pragma once
#include "core/math.h"

struct FemalePose {
    Vector3 root;
    Vector3 head;
    Vector3 lHand, rHand;
    Vector3 lFoot, rFoot;
    Vector3 lKneePole, rKneePole;
    Vector3 lElbowPole, rElbowPole;
    float yaw;
    float breathOffset;
    Vector3 lHairPos, rHairPos;
    Vector3 lChainPos, rChainPos;
};

struct AnimState {
    float time;
    int state;
    int prevState;
    float transition;
    float blinkTimer;
    float blinkAmount;
    Vector3 lHairPos, lHairVel;
    Vector3 rHairPos, rHairVel;
    Vector3 lChainPos, lChainVel;
    Vector3 rChainPos, rChainVel;
    Vector3 velocity;
    float moveSpeed;
    bool focusMode;
};