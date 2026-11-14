#pragma once
#include "raylib.h"

struct MultiTouchState {
    Vector2 dragStart;
    Vector2 dragCurrent;
    Vector2 dragDelta;
    bool isDragging;
    int dragTouchId;
    Vector2 tappedPos;
    bool justTapped;
};

inline void InitMultiTouch(MultiTouchState& state) {
    state.isDragging = false;
    state.dragTouchId = -1;
    state.justTapped = false;
}

inline bool IsButtonJustTapped(const MultiTouchState& state, Rectangle rect) {
    return state.justTapped && CheckCollisionPointRec(state.tappedPos, rect);
}

inline void EndMultiTouchFrame(MultiTouchState& state) {
    state.justTapped = false;
    state.dragDelta = {0, 0};
}