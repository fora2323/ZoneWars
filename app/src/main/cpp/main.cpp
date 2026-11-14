#include "raylib.h"
#include "sausage.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(0, 0, "SausageWars");
    SetTargetFPS(60);

    Player pl = {0};
    pl.pos = {0, 0, 0};
    pl.pitch = 0.2f;
    pl.state = 0;

    Camera3D cam = {0};
    cam.up = {0, 1, 0};
    cam.fovy = 55.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle btnJump = {(float)sw - 140, (float)sh - 260, 110, 110};
        Rectangle btnCrouch = {(float)sw - 270, (float)sh - 140, 110, 110};
        Rectangle btnProne = {(float)sw - 140, (float)sh - 140, 110, 110};

        Vector2 joyCenter = {150, (float)sh - 150};
        float joyRadius = 100.0f;
        Vector2 joyInput = {0, 0};
        
        bool tapJump = false;
        bool tCrouch = false;
        bool tProne = false;
        
        bool isDragging = false;
        Vector2 dragDelta = {0, 0};
        
        if (IsKeyDown(KEY_W)) joyInput.y = -1;
        if (IsKeyDown(KEY_S)) joyInput.y = 1;
        if (IsKeyDown(KEY_A)) joyInput.x = -1;
        if (IsKeyDown(KEY_D)) joyInput.x = 1;
        if (IsKeyPressed(KEY_SPACE)) tapJump = true;
        if (IsKeyPressed(KEY_C)) tCrouch = true;
        if (IsKeyPressed(KEY_Z)) tProne = true;

        for (int i = 0; i < GetTouchPointCount(); ++i) {
            Vector2 tp = GetTouchPosition(i);
            
            if (CheckCollisionPointRec(tp, btnJump)) {
                if (IsGestureDetected(GESTURE_TAP) || GetTouchPointId(i) >= 0) tapJump = true;
            } else if (CheckCollisionPointRec(tp, btnCrouch)) {
                tCrouch = true;
            } else if (CheckCollisionPointRec(tp, btnProne)) {
                tProne = true;
            } else if (tp.x < sw / 2.0f) {
                Vector2 d = {tp.x - joyCenter.x, tp.y - joyCenter.y};
                float len = sqrtf(d.x * d.x + d.y * d.y);
                if (len > 0) {
                    joyInput = {d.x / len, d.y / len};
                    if (len < joyRadius) joyInput = {joyInput.x * (len / joyRadius), joyInput.y * (len / joyRadius)};
                }
            } else {
                Vector2 delta = GetTouchPosition(i);
                delta.x -= joyCenter.x; 
                isDragging = true;
                dragDelta = GetGestureDragVector();
            }
        }
        
        if (isDragging) {
            pl.yaw -= dragDelta.x * 0.005f;
            pl.pitch -= dragDelta.y * 0.005f;
        }
        pl.pitch = Clampf(pl.pitch, -0.5f, 1.2f);

        if (tapJump) {
            double t = GetTime();
            if (t - pl.lastJumpTime < 0.35 && pl.jumpCount == 1) {
                pl.vel.y = 7.5f;
                pl.isSpinning = true;
                pl.spinAngle = 0.0f;
                pl.jumpCount = 2;
                pl.state = 4;
            } else if (pl.pos.y <= 0.01f) {
                pl.vel.y = 7.0f;
                pl.jumpCount = 1;
                pl.lastJumpTime = t;
                pl.state = 4;
            }
        }

        if (tProne) {
            pl.state = (pl.state == 3) ? 0 : 3;
        } else if (tCrouch && pl.state != 3) {
            pl.state = (pl.state == 2) ? 0 : 2;
        } else if (pl.pos.y > 0.01f) {
            pl.state = 4;
        }

        if (pl.pos.y > 0.01f) {
            pl.vel.y -= 20.0f * dt;
        }

        if (pl.isSpinning) {
            pl.spinAngle -= 14.0f * dt;
        }

        float speed = 0.0f;
        if (pl.state == 0 || pl.state == 4) speed = 6.0f;
        if (pl.state == 2) speed = 2.5f;
        if (pl.state == 3) speed = 1.0f;

        float joyMag = sqrtf(joyInput.x * joyInput.x + joyInput.y * joyInput.y);
        if (joyMag > 0.1f) {
            float targetYaw = pl.yaw + atan2f(-joyInput.x, -joyInput.y);
            pl.yaw = Lerpf(pl.yaw, targetYaw, dt * 10.0f);
            pl.vel.x = sinf(pl.yaw) * speed * joyMag;
            pl.vel.z = cosf(pl.yaw) * speed * joyMag;
            if (pl.state == 0) pl.state = 1;
        } else {
            pl.vel.x = Lerpf(pl.vel.x, 0.0f, dt * 15.0f);
            pl.vel.z = Lerpf(pl.vel.z, 0.0f, dt * 15.0f);
            if (pl.state == 1) pl.state = 0;
        }

        pl.pos = VAdd(pl.pos, VMul(pl.vel, dt));
        
        if (pl.pos.y <= 0.0f) {
            pl.pos.y = 0.0f;
            pl.vel.y = 0.0f;
            pl.jumpCount = 0;
            pl.isSpinning = false;
            pl.spinAngle = 0.0f;
            if (pl.state == 4) pl.state = 0;
        }

        Vector3 camOffset = {0, 1.8f, 0};
        if (pl.state == 2) camOffset.y = 1.2f;
        if (pl.state == 3) camOffset.y = 0.6f;

        cam.target = VAdd(pl.pos, camOffset);
        float cd = 3.5f;
        Vector3 sph = {cd * cosf(pl.pitch) * sinf(pl.yaw), cd * sinf(pl.pitch), cd * cosf(pl.pitch) * cosf(pl.yaw)};
        cam.position = VAdd(cam.target, sph);

        BeginDrawing();
        ClearBackground({135, 206, 235, 255});
        
        BeginMode3D(cam);
        DrawPolygonWorld();
        DrawSausageMan(pl);
        EndMode3D();

        DrawCircleV(joyCenter, joyRadius, {50, 50, 50, 100});
        DrawCircleV(VAdd(joyCenter, {joyInput.x * joyRadius, joyInput.y * joyRadius}), 35, {200, 200, 200, 150});

        DrawUIButton(btnJump, "LOMPAT", pl.state == 4);
        DrawUIButton(btnCrouch, "JONGKOK", pl.state == 2);
        DrawUIButton(btnProne, "TIARAP", pl.state == 3);

        DrawText("SausageWars", 20, 20, 30, BLACK);
        DrawFPS(20, sh - 40);

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}