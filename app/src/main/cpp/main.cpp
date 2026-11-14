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

float v_buffer[MAX_VERTICES * 3];
float n_buffer[MAX_VERTICES * 3];
unsigned char c_buffer[MAX_VERTICES * 4];
unsigned short i_buffer[MAX_INDICES];
Mesh gCharMesh;
Material gCharMat;
int gVtxCount;
int gIdxCount;

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
    
    float curFov = 48.0f;
    float curCamDist = 2.15f;
    float curCamHeight = 1.44f;
    
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
        
        Vector2 joyCenter = {120, (float)sh - 120};
        float joyRadius = 88.0f;
        Vector2 joyInput = {0, 0};
        
        if (IsKeyDown(KEY_W)) joyInput.y = -1;
        if (IsKeyDown(KEY_S)) joyInput.y = 1;
        if (IsKeyDown(KEY_A)) joyInput.x = -1;
        if (IsKeyDown(KEY_D)) joyInput.x = 1;
        
        ts.justTapped = false;
        ts.isDragging = false;
        for (int i = 0; i < GetTouchPointCount(); ++i) {
            Vector2 tp = GetTouchPosition(i);
            if (tp.x < sw / 2.0f && !IsPointInAnyUI(tp, uiRects, 6)) {
                Vector2 d = {tp.x - joyCenter.x, tp.y - joyCenter.y};
                float len = sqrtf(d.x*d.x + d.y*d.y);
                if (len > 0) {
                    joyInput = {d.x / len, d.y / len};
                    if (len < joyRadius) joyInput = {joyInput.x * (len / joyRadius), joyInput.y * (len / joyRadius)};
                }
            } else if (!IsPointInAnyUI(tp, uiRects, 6)) {
                if (!ts.isDragging) {
                    ts.isDragging = true;
                    ts.dragTouchId = i;
                    ts.dragStart = tp;
                    ts.dragCurrent = tp;
                } else if (ts.dragTouchId == i) {
                    ts.dragDelta = {tp.x - ts.dragCurrent.x, tp.y - ts.dragCurrent.y};
                    ts.dragCurrent = tp;
                }
            }
        }
        
        if (IsMouseButtonPressed(0)) {
            ts.justTapped = true;
            ts.tappedPos = GetMousePosition();
        }
        for(int i=0; i<GetTouchPointCount(); ++i) {
            if (GetTouchPointCount() > 0 && !ts.isDragging) {
                ts.justTapped = true;
                ts.tappedPos = GetTouchPosition(i);
            }
        }
        
        if (IsButtonJustTapped(ts, uiRects[0]) || IsKeyPressed(KEY_V)) pl.camMode = (pl.camMode + 1) % 4;
        if (IsButtonJustTapped(ts, uiRects[1]) || IsKeyPressed(KEY_F)) pl.anim.focusMode = !pl.anim.focusMode;
        
        bool wRun = IsKeyDown(KEY_LEFT_SHIFT);
        bool wCrouch = IsKeyDown(KEY_C);
        bool wSlide = IsKeyDown(KEY_LEFT_CONTROL);
        bool wJump = IsKeyDown(KEY_SPACE);
        
        for (int i = 0; i < GetTouchPointCount(); ++i) {
            Vector2 tp = GetTouchPosition(i);
            if (CheckCollisionPointRec(tp, uiRects[2])) wRun = true;
            if (CheckCollisionPointRec(tp, uiRects[5])) wCrouch = true;
            if (CheckCollisionPointRec(tp, uiRects[4])) wSlide = true;
            if (CheckCollisionPointRec(tp, uiRects[3])) wJump = true;
        }
        
        float joyMag = sqrtf(joyInput.x*joyInput.x + joyInput.y*joyInput.y);
        if (joyMag > 1.0f) joyInput = {joyInput.x / joyMag, joyInput.y / joyMag};
        
        float sens = pl.anim.focusMode ? 0.0032f : 0.0048f;
        if (ts.isDragging) {
            pl.yaw -= ts.dragDelta.x * sens;
            pl.pitch -= ts.dragDelta.y * sens;
        }
        if (IsKeyDown(KEY_Q)) pl.yaw += 2.0f * dt;
        if (IsKeyDown(KEY_E)) pl.yaw -= 2.0f * dt;
        pl.pitch = Clampf(pl.pitch, -0.30f, 0.85f);
        
        if (pl.slideTimer > 0.0f) {
            pl.slideTimer -= dt;
            float slideSpeed = 9.0f * (1.0f - (0.72f - pl.slideTimer) * 2.5f);
            if (slideSpeed < 0) slideSpeed = 0;
            pl.anim.velocity = VMul(pl.slideDir, slideSpeed);
            pl.anim.state = 4;
            if (pl.slideTimer <= 0.0f) wCrouch = true;
        } else if (wJump && pl.pos.y <= 0.01f) {
            pl.anim.velocity.y = 6.5f;
            if (pl.anim.state == 2) pl.anim.velocity = VAdd(pl.anim.velocity, VMul(RotateYaw({0, 0, 1}, pl.yaw), 6.8f));
            pl.anim.state = 5;
        } else if (pl.pos.y > 0.01f) {
            pl.anim.velocity.y -= 16.0f * dt;
            pl.anim.state = 5;
        } else if (wSlide && pl.anim.state == 2) {
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
            
            if (joyMag > 0.1f) {
                float targetYaw = pl.yaw + atan2f(-joyInput.x, -joyInput.y);
                if (pl.anim.focusMode) {
                    pl.anim.velocity = RotateYaw({joyInput.x, 0, joyInput.y}, pl.yaw);
                    pl.anim.velocity = VMul(VNorm(pl.anim.velocity), pl.anim.moveSpeed);
                } else {
                    pl.yaw = targetYaw;
                    pl.anim.velocity = VMul(RotateYaw({0, 0, 1}, pl.yaw), pl.anim.moveSpeed);
                }
            } else {
                pl.anim.velocity = {0, 0, 0};
            }
            
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
        else if (pl.camMode == 2) { tDist = 1.45f; tFov = 42.0f; tHeight = 1.52f; camOffset.z = -1.0f; }
        else if (pl.camMode == 3) { tDist = 2.85f; camOffset.x = 0; }
        
        curFov = Lerpf(curFov, tFov, dt * 10.0f);
        curCamDist = Lerpf(curCamDist, tDist, dt * 10.0f);
        curCamHeight = Lerpf(curCamHeight, tHeight, dt * 14.0f);
        
        cam.fovy = curFov;
        cam.target = VAdd(pl.pos, {0, curCamHeight, 0});
        Basis3D cb = MakeBasis(RotateYaw({0, 0, 1}, pl.yaw));
        cam.target = VAdd(cam.target, VMul(cb.right, camOffset.x));
        
        float cy = pl.yaw + (pl.camMode == 2 ? M_PI : 0);
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
        
        DrawCircleV(joyCenter, joyRadius, {100, 100, 100, 100});
        DrawCircleV(VAdd({joyCenter.x, joyCenter.y, 0}, {joyInput.x * joyRadius, joyInput.y * joyRadius, 0}), 30, {200, 200, 200, 200});
        
        if (pl.anim.focusMode) {
            DrawLine(sw/2 - 10, sh/2, sw/2 + 10, sh/2, WHITE);
            DrawLine(sw/2, sh/2 - 10, sw/2, sh/2 + 10, WHITE);
        }
        
        DrawText("ZoneWars 3D", 20, 20, 30, WHITE);
        DrawFPS(20, sh - 40);
        
        EndDrawing();
        EndMultiTouchFrame(ts);
    }
    
    CloseWindow();
    return 0;
}