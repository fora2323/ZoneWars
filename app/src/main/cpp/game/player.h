#pragma once
#include "anim/anim_state.h"

struct Player {
    Vector3 pos;
    float yaw;
    float pitch;
    AnimState anim;
    int camMode;
    float slideTimer;
    Vector3 slideDir;
};