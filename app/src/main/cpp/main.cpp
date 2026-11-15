#include "raylib.h"
#include "sausage.h"
#include "map/arena-map1.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(0, 0, "ZoneWars");
    SetTargetFPS(60);

    Player pl = {0};
    pl.pitch = 0.2f;

    Camera3D cam = {0};
    cam.up = {0, 1, 0};
    cam.fovy = 55.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    bool prevJump = false, prevCrouch = false, prevProne = false;
    bool lookValid = false;
    Vector2 lookPrev = {0, 0};

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle btnJump   = {(float)sw - 140, (float)sh - 260, 110, 110};
        Rectangle btnCrouch = {(float)sw - 270, (float)sh - 140, 110, 110};
        Rectangle btnProne  = {(float)sw - 140, (float)sh - 140, 110, 110};
        Vector2 joyCenter = {150, (float)sh - 150};
        const float joyRadius = 100.0f;

        // Kumpulkan titik sentuh (mouse dianggap sentuhan untuk tes di PC)
        Vector2 pts[10];
        int nPts = 0;
        int touches = GetTouchPointCount();
        for (int i = 0; i < touches && nPts < 10; ++i) pts[nPts++] = GetTouchPosition(i);
        if (touches == 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) pts[nPts++] = GetMousePosition();

        Vector2 joyInput = {0, 0};
        if (IsKeyDown(KEY_W)) joyInput.y -= 1.0f;   // maju
        if (IsKeyDown(KEY_S)) joyInput.y += 1.0f;   // mundur
        if (IsKeyDown(KEY_A)) joyInput.x -= 1.0f;
        if (IsKeyDown(KEY_D)) joyInput.x += 1.0f;

        bool jumpDown = false, crouchDown = false, proneDown = false;
        bool lookDown = false;
        Vector2 lookPos = {0, 0};

        for (int i = 0; i < nPts; ++i) {
            Vector2 tp = pts[i];
            if (CheckCollisionPointRec(tp, btnJump)) {
                jumpDown = true;
            } else if (CheckCollisionPointRec(tp, btnCrouch)) {
                crouchDown = true;
            } else if (CheckCollisionPointRec(tp, btnProne)) {
                proneDown = true;
            } else if (tp.x < sw / 2.0f) {
                Vector2 d = {tp.x - joyCenter.x, tp.y - joyCenter.y};
                float len = sqrtf(d.x * d.x + d.y * d.y);
                if (len > 0.0f) {
                    float mag = fminf(len / joyRadius, 1.0f);
                    joyInput = {d.x / len * mag, d.y / len * mag};
                }
            } else if (!lookDown) {
                lookDown = true;
                lookPos = tp;
            }
        }

        // Pandangan kamera: geser layar kanan atau drag mouse kanan
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 md = GetMouseDelta();
            pl.yaw -= md.x * 0.005f;
            pl.pitch -= md.y * 0.005f;
        }
        if (lookDown) {
            if (lookValid) {
                pl.yaw   -= (lookPos.x - lookPrev.x) * 0.008f;
                pl.pitch -= (lookPos.y - lookPrev.y) * 0.008f;
            }
            lookPrev = lookPos;
            lookValid = true;
        } else {
            lookValid = false;
        }
        pl.pitch = Clampf(pl.pitch, -0.5f, 1.2f);

        // Tombol dengan deteksi tekan sekali (bukan terus-menerus)
        bool tapJump = IsKeyPressed(KEY_SPACE) || (jumpDown && !prevJump);
        bool tCrouch = IsKeyPressed(KEY_C) || (crouchDown && !prevCrouch);
        bool tProne  = IsKeyPressed(KEY_Z) || (proneDown && !prevProne);
        prevJump = jumpDown;
        prevCrouch = crouchDown;
        prevProne = proneDown;

        bool onGround = pl.pos.y <= 0.001f;

        // Lompat & lompat ganda (tidak bisa saat tiarap)
        if (tapJump && pl.state != 3) {
            double t = GetTime();
            if (!onGround && pl.jumpCount == 1 && t - pl.lastJumpTime < 0.4) {
                pl.vel.y = 7.5f;
                pl.isSpinning = true;
                pl.spinAngle = 0.0f;
                pl.jumpCount = 2;
            } else if (onGround) {
                pl.vel.y = 7.0f;
                pl.jumpCount = 1;
                pl.lastJumpTime = t;
            }
        }

        // Jongkok & tiarap (hanya di tanah)
        if (tProne && onGround) {
            pl.state = (pl.state == 3) ? 0 : 3;
        } else if (tCrouch && onGround && pl.state != 3) {
            pl.state = (pl.state == 2) ? 0 : 2;
        }
        if (!onGround) pl.state = 4;

        // Kecepatan sesuai posisi
        float mag = sqrtf(joyInput.x * joyInput.x + joyInput.y * joyInput.y);
        if (mag > 1.0f) {
            joyInput.x /= mag;
            joyInput.y /= mag;
            mag = 1.0f;
        }
        float speed = 6.0f;
        if (pl.state == 2) speed = 2.5f;
        if (pl.state == 3) speed = 1.3f;

        // Arah gerak relatif ke kamera
        float sy = sinf(pl.yaw), cy = cosf(pl.yaw);
        Vector3 fwd   = {sy, 0, cy};
        Vector3 right = {-cy, 0, sy};
        Vector3 target = {0, 0, 0};
        if (mag > 0.1f) {
            Vector3 wish = VAdd(VMul(fwd, -joyInput.y), VMul(right, joyInput.x));
            float wl = sqrtf(wish.x * wish.x + wish.z * wish.z);
            if (wl > 0.0001f) wish = {wish.x / wl, 0, wish.z / wl};
            target = VMul(wish, speed * mag);
        }
        float k = onGround ? 14.0f : 3.0f;
        pl.vel.x = Lerpf(pl.vel.x, target.x, dt * k);
        pl.vel.z = Lerpf(pl.vel.z, target.z, dt * k);

        // Karakter menghadap arah gerak (sudut terpendek)
        float hspeed = sqrtf(pl.vel.x * pl.vel.x + pl.vel.z * pl.vel.z);
        if (hspeed > 0.3f) {
            float tYaw = atan2f(pl.vel.x, pl.vel.z);
            float diff = atan2f(sinf(tYaw - pl.faceYaw), cosf(tYaw - pl.faceYaw));
            pl.faceYaw += diff * fminf(1.0f, dt * 12.0f);
        }

        // Animasi lari
        float targetMove = Clampf(hspeed / 6.0f, 0.0f, 1.0f);
        pl.moveAmount = Lerpf(pl.moveAmount, targetMove, dt * 10.0f);
        if (onGround) {
            pl.walkPhase = fmodf(pl.walkPhase + hspeed * dt * 1.2f, 6.2831853f);
        }
        pl.crouchBlend = Lerpf(pl.crouchBlend, pl.state == 2 ? 1.0f : 0.0f, dt * 12.0f);
        pl.proneBlend  = Lerpf(pl.proneBlend,  pl.state == 3 ? 1.0f : 0.0f, dt * 10.0f);

        // Fisika
        float prevY = pl.pos.y;
        if (pl.pos.y > 0.0f) pl.vel.y -= 20.0f * dt;
        if (pl.isSpinning) pl.spinAngle -= 14.0f * dt;

        pl.pos = VAdd(pl.pos, VMul(pl.vel, dt));

        if (pl.pos.y <= 0.0f) {
            pl.pos.y = 0.0f;
            if (pl.vel.y < 0.0f) pl.vel.y = 0.0f;
            if (prevY > 0.0f) {   // baru mendarat
                pl.jumpCount = 0;
                pl.isSpinning = false;
                pl.spinAngle = 0.0f;
                if (pl.state == 4) pl.state = 0;
            }
        }

        // Kamera di belakang karakter
        float camH = Lerpf(Lerpf(1.25f, 0.85f, pl.crouchBlend), 0.5f, pl.proneBlend);
        cam.target = VAdd(pl.pos, {0, camH, 0});
        float cd = 4.0f;
        Vector3 back = {
            -sinf(pl.yaw) * cosf(pl.pitch),
             sinf(pl.pitch),
            -cosf(pl.yaw) * cosf(pl.pitch)
        };
        cam.position = VAdd(cam.target, VMul(back, cd));

        BeginDrawing();
        ClearBackground({135, 206, 235, 255});

        BeginMode3D(cam);
        DrawPolygonWorld();
        DrawSausageMan(pl);
        EndMode3D();

        // Joystick
        Vector2 knob = {joyCenter.x + joyInput.x * joyRadius, joyCenter.y + joyInput.y * joyRadius};
        DrawCircleV(joyCenter, joyRadius, {50, 50, 50, 100});
        DrawCircleV(knob, 35, {200, 200, 200, 150});

        DrawUIButton(btnJump,   "LOMPAT",   pl.state == 4);
        DrawUIButton(btnCrouch, "JONGKOK",  pl.state == 2);
        DrawUIButton(btnProne,  "TIARAP",   pl.state == 3);

        DrawText("ZoneWars", 20, 20, 30, BLACK);
        DrawFPS(20, sh - 40);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}