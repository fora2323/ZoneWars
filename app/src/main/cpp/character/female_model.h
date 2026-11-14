#pragma once
#include "character/female_body.h"
#include "character/female_head.h"
#include "render/shadow.h"

inline void DrawFemaleCharacter(const FemalePose& p, Vector3 pos, float yaw, float blink) {
    DrawDropShadow(pos, 0.35f);
    DrawFemaleBody(p);
    DrawFemaleHead(p, blink);
}