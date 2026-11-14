#include "raylib.h"
#include "core/math.h"
#include "core/touch.h"
#include "anim/pose.h"
#include "anim/anim_state.h"
#include "anim/anim_pose.h"
#include "render/shading.h"
#include "character/female_model.h"
#include "ui/ui.h"
#include "world/arena.h"
#include "game/player.h"

Mesh gArenaMesh;
Material gArenaMat;

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(0, 0, "ZoneWars");
    SetTargetFPS(60);

    InitDynamicMesh();
    InitArenaMesh();

    Player pl;
    pl.pos = {0, 0, 0};
    pl.yaw = 0;
    pl.pitch = 0.2f;
    InitAnim(pl.anim);
    pl.camMode = 0;
    pl.slideTimer = 0.0f;
    pl.slideDir = {0, 0, 1};

    MultiTouchState ts;
    InitMultiTouch(ts);

    Camera3D cam = {0};
    cam.up = {0, 1, 0};
    cam.fovy = 48.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    float camYaw = 0.0f;
    float curFov = 48.0f;
    float curCamDist = 2.15f;
    float curCamHeight = 1.44f;

    int joyId = -1;
    Vector2 joyOrigin = {0, 0};
    int prevTouchIds[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int prevTouchCount = 0;
    const float joyRadius = 88.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle uiRects[] = {
            {(float)sw - 215, 18, 195, 44},
            {(float)sw - 265, (float)sh - 220, 118, 56},
            {(float)sw - 135, (float)sh - 294, 118, 56},
            {(float)sw - 135, (float)sh - 220, 118, 56},
            {(float)sw - 265, (float)sh - 146, 118, 56},
            {(float)sw - 135, (float)sh - 146, 118, 56}
        };

        int touchCount = GetTouchPointCount();
        if (touchCount > 10) touchCount = 10;
        int curIds[10];
        for (int i = 0; i < touchCount; ++i) curIds[i] = GetTouchPointId(i);

        bool joyHeld = false;
        Vector2 joyTouch = joyOrigin;
        for (int i = 0; i < touchCount; ++i) {
            int id = curIds[i];
            Vector2 tp = GetTouchPosition(i);
            if (tp.x >= sw / 2.0f || IsPointInAnyUI(tp, uiRects, 6)) continue;
            if (joyId == -1) {
                joyId = id;
                joyOrigin = tp;
            }
            if (joyId == id) {
                joyTouch = tp;
                joyHeld = true;
            }
        }
        if (!joyHeld) joyId = -1;

        Vector2 joyInput = {0, 0};
        if (joyHeld) {
            Vector2 d = {joyTouch.x - joyOrigin.x, joyTouch.y - joyOrigin.y};
            float len = sqrtf(d.x * d.x + d.y * d.y);
            if (len > joyRadius) {
                d.x *= joyRadius / len;
                d.y *= joyRadius / len;
            }
            joyInput = {d.x / joyRadius, d.y / joyRadius};
        }

        if (IsKeyDown(KEY_W)) joyInput.y -= 1.0f;
        if (IsKeyDown(KEY_S)) joyInput.y += 1.0f;
        if (IsKeyDown(KEY_A)) joyInput.x -= 1.0f;
        if (IsKeyDown(KEY_D)) joyInput.x += 1.0f;

        float joyMag = sqrtf(joyInput.x * joyInput.x + joyInput.y * joyInput.y);
        if (joyMag > 1.0f) {
            joyInput.x /= joyMag;
            joyInput.y /= joyMag;
            joyMag = 1.0f;
        }

        ts.dragDelta = {0, 0};
        bool dragFound = false;
        for (int i = 0; i < touchCount; ++i) {
            int id = curIds[i];
            Vector2 tp = GetTouchPosition(i);
            if (tp.x < sw / 2.0f || IsPointInAnyUI(tp, uiRects, 6)) continue;
            if (!ts.isDragging) {
                ts.isDragging = true;
                ts.dragTouchId = id;
                ts.dragStart = tp;
                ts.dragCurrent = tp;
                dragFound = true;
            } else if (ts.dragTouchId == id) {
                ts.dragDelta = {tp.x - ts.dragCurrent.x, tp.y - ts.dragCurrent.y};
                ts.dragCurrent = tp;
                dragFound = true;
            }
        }
        if (!dragFound) {
            ts.isDragging = false;
            ts.dragTouchId = -1;
        }

        ts.justTapped = false;
        for (int i = 0; i < touchCount; ++i) {
            bool isNew = true;
            for (int j = 0; j < prevTouchCount; ++j) {
                if (prevTouchIds[j] == curIds[i]) {
                    isNew = false;
                    break;
                }
            }
            if (isNew) {
                ts.justTapped = true;
                ts.tappedPos = GetTouchPosition(i);
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ts.justTapped = true;
            ts.tappedPos = GetMousePosition();
        }

        if (IsButtonJustTapped(ts, uiRects[0]) || IsKeyPressed(KEY_V)) pl.camMode = (pl.camMode + 1) % 4;
        if (IsButtonJustTapped(ts, uiRects[1]) || IsKeyPressed(KEY_F)) pl.anim.focusMode = !pl.anim.focusMode;

        bool wRun = IsKeyDown(KEY_LEFT_SHIFT);
        bool wCrouch = IsKeyDown(KEY_C);
        bool wSlideTap = IsButtonJustTapped(ts, uiRects[4]) || IsKeyPressed(KEY_LEFT_CONTROL);
        bool wJumpTap = IsButtonJustTapped(ts, uiRects[3]) || IsKeyPressed(KEY_SPACE);
        for (int i = 0; i < touchCount; ++i) {
            Vector2 tp = GetTouchPosition(i);
            if (CheckCollisionPointRec(tp, uiRects[2])) wRun = true;
            if (CheckCollisionPointRec(tp, uiRects[5])) wCrouch = true;
        }

        float sens = pl.anim.focusMode ? 0.0032f : 0.0048f;
        if (ts.isDragging) {
            camYaw -= ts.dragDelta.x * sens;
            pl.pitch -= ts.dragDelta.y * sens;
        }
        if (IsKeyDown(KEY_Q)) camYaw += 2.0f * dt;
        if (IsKeyDown(KEY_E)) camYaw -= 2.0f * dt;
        pl.pitch = Clampf(pl.pitch, -0.30f, 0.85f);

        if (pl.slideTimer > 0.0f) {
            pl.slideTimer -= dt;
            float slideSpeed = 9.0f * (1.0f - (0.72f - pl.slideTimer) * 2.5f);
            if (slideSpeed < 0) slideSpeed = 0;
            pl.anim.velocity = VMul(pl.slideDir, slideSpeed);
            pl.anim.state = 4;
            if (pl.slideTimer <= 0.0f) wCrouch = true;
        } else if (wJumpTap && pl.pos.y <= 0.01f) {
            pl.anim.velocity.y = 6.5f;
            if (pl.anim.state == 2) pl.anim.velocity = VAdd(pl.anim.velocity, VMul(RotateYaw({0, 0, 1}, pl.yaw), 6.8f));
            pl.anim.state = 5;
        } else if (pl.pos.y > 0.01f) {
            pl.anim.velocity.y -= 16.0f * dt;
            pl.anim.state = 5;
        } else if (wSlideTap && pl.anim.state == 2) {
            pl.slideTimer = 0.72f;
            pl.slideDir = RotateYaw({0, 0, 1}, pl.yaw);
            pl.anim.state = 4;
        } else {
            pl.anim.velocity.y = 0;
            pl.pos.y = 0;
            float targetSpeed = 0.0f;
            if (joyMag > 0.1f) {
                targetSpeed = 3.1f;
                if ((joyMag > 0.72f || wRun) && !wCrouch && !pl.anim.focusMode) targetSpeed = 6.4f;
                if (wCrouch || pl.anim.focusMode) targetSpeed = 1.95f;
            }
            pl.anim.moveSpeed = Lerpf(pl.anim.moveSpeed, targetSpeed, dt * 11.0f);

            Vector3 moveDir = VAdd(VMul(RotateYaw({0, 0, 1}, camYaw), -joyInput.y),
                                   VMul(RotateYaw({1, 0, 0}, camYaw), joyInput.x));
            if (joyMag > 0.1f) {
                pl.anim.velocity = VMul(VNorm(moveDir), pl.anim.moveSpeed);
            } else {
                pl.anim.velocity = {0, 0, 0};
            }

            float desiredYaw = pl.yaw;
            if (pl.anim.focusMode) desiredYaw = camYaw + (float)M_PI;
            else if (joyMag > 0.1f) desiredYaw = atan2f(moveDir.x, moveDir.z);
            pl.yaw += WrapAngle(desiredYaw - pl.yaw) * fminf(dt * 12.0f, 1.0f);

            if (wCrouch) pl.anim.state = 3;
            else if (pl.anim.moveSpeed > 4.5f) pl.anim.state = 2;
            else if (pl.anim.moveSpeed > 0.5f) pl.anim.state = 1;
            else pl.anim.state = 0;
        }

        pl.pos = VAdd(pl.pos, VMul(pl.anim.velocity, dt));
        pl.pos.x = Clampf(pl.pos.x, -58.0f, 58.0f);
        pl.pos.z = Clampf(pl.pos.z, -58.0f, 58.0f);
        if (pl.pos.y < 0) pl.pos.y = 0;

        UpdateAnim(pl.anim, dt, pl.pos, pl.yaw);
        FemalePose pose = ComputeFemalePose(pl.anim, pl.pos, pl.yaw);

        float tFov = 48.0f, tDist = 2.15f, tHeight = 1.44f;
        Vector3 camOffset = {0, 0, 0};

        if (pl.anim.focusMode) {
            tFov = 40.0f; tDist = 1.40f; camOffset.x = 0.40f;
        } else if (pl.anim.state == 4) {
            tFov = 56.0f; tDist = 2.55f; tHeight = 0.8f;
        } else if (pl.anim.state == 2) {
            tFov = 54.0f; tDist = 2.50f;
        }

        if (pl.camMode == 0) { camOffset.x = 0.34f; }
        else if (pl.camMode == 1) { camOffset.x = -0.34f; }
        else if (pl.camMode == 2) { tDist = 1.45f; tFov = 42.0f; tHeight = 1.52f; }
        else if (pl.camMode == 3) { tDist = 2.85f; camOffset.x = 0; }

        curFov = Lerpf(curFov, tFov, dt * 10.0f);
        curCamDist = Lerpf(curCamDist, tDist, dt * 10.0f);
        curCamHeight = Lerpf(curCamHeight, tHeight, dt * 14.0f);

        cam.fovy = curFov;
        Basis3D cb = MakeBasis(RotateYaw({0, 0, 1}, camYaw));
        cam.target = VAdd(VAdd(pl.pos, {0, curCamHeight, 0}), VMul(cb.right, camOffset.x));

        float cy = (pl.camMode == 2) ? pl.yaw : camYaw;
        Vector3 sph = { curCamDist * cosf(pl.pitch) * sinf(cy), curCamDist * sinf(pl.pitch), curCamDist * cosf(pl.pitch) * cosf(cy) };
        cam.position = VAdd(cam.target, sph);

        BeginDrawing();
        ClearBackground({118, 152, 192, 255});

        BeginMode3D(cam);
        DrawFlatArenaMap(pl.pos);

        BeginDynamicMesh();
        DrawFemaleCharacter(pose, pl.pos, pl.yaw, BlinkAmount(pl.anim));
        EndDynamicMesh();

        EndMode3D();

        const char* camLabels[] = {"BAHU: KANAN", "BAHU: KIRI", "KAMERA: DEPAN", "KAMERA: TENGAH"};
        DrawUIButton(uiRects[0], camLabels[pl.camMode], true, {200, 50, 50, 255});
        DrawUIButton(uiRects[1], "FOKUS", pl.anim.focusMode, {200, 50, 50, 255});
        DrawUIButton(uiRects[2], "LARI", pl.anim.state == 2, {200, 50, 50, 255});
        DrawUIButton(uiRects[3], "LOMPAT", pl.anim.state == 5, {200, 50, 50, 255});
        DrawUIButton(uiRects[4], "SLIDE", pl.anim.state == 4, {200, 50, 50, 255});
        DrawUIButton(uiRects[5], "JONGKOK", pl.anim.state == 3, {200, 50, 50, 255});

        if (joyHeld) {
            DrawCircleV(joyOrigin, joyRadius, Color{100, 100, 100, 100});
            DrawCircleV(Vector2{ joyOrigin.x + joyInput.x * joyRadius, joyOrigin.y + joyInput.y * joyRadius }, 30.0f, Color{200, 200, 200, 200});
        }

        if (pl.anim.focusMode) {
            DrawLine(sw / 2 - 10, sh / 2, sw / 2 + 10, sh / 2, WHITE);
            DrawLine(sw / 2, sh / 2 - 10, sw / 2, sh / 2 + 10, WHITE);
        }

        DrawText("ZoneWars 3D", 20, 20, 30, WHITE);
        DrawFPS(20, sh - 40);

        EndDrawing();

        prevTouchCount = touchCount;
        for (int i = 0; i < touchCount; ++i) prevTouchIds[i] = curIds[i];
        EndMultiTouchFrame(ts);
    }

    CloseWindow();
    return 0;
}