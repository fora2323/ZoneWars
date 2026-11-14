#include "raylib.h"
#include "character.h"
#include "female_character.h"
#include <cmath>

struct Player {
    Vector3 pos;
    Vector3 vel;
    float yaw;
    bool female;
};

static float Clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static float WrapAngle(float a)
{
    const float twoPi = 6.2831853f;
    return a - twoPi * roundf(a / twoPi);
}

int main(void)
{
    InitWindow(0, 0, "ZoneWars");
    SetTargetFPS(60);

    Camera3D cam = { 0 };
    cam.up = V3(0.0f, 1.0f, 0.0f);
    cam.fovy = 55.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    Player p = { V3(0.0f, 0.0f, 0.0f), V3(0.0f, 0.0f, 0.0f), 0.0f, true };

    AnimState anim;
    InitAnim(anim);
    Pose pose;

    float camYaw = 0.0f;
    float camPitch = 0.4f;
    const float camDist = 4.5f;
    const float joyRadius = 90.0f;
    const float moveSpeed = 3.5f;

    int joyId = -1;
    Vector2 joyOrigin = Vector2{ 0.0f, 0.0f };
    Vector2 joyKnob = Vector2{ 0.0f, 0.0f };
    int lookId = -1;
    Vector2 lookLast = Vector2{ 0.0f, 0.0f };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        Rectangle switchBtn = Rectangle{ (float)sw - 190.0f, 20.0f, 170.0f, 50.0f };

        float ix = 0.0f;
        float iz = 0.0f;
        if (IsKeyDown(KEY_W)) iz += 1.0f;
        if (IsKeyDown(KEY_S)) iz -= 1.0f;
        if (IsKeyDown(KEY_D)) ix += 1.0f;
        if (IsKeyDown(KEY_A)) ix -= 1.0f;
        if (IsKeyDown(KEY_Q)) camYaw += 1.5f * dt;
        if (IsKeyDown(KEY_E)) camYaw -= 1.5f * dt;

        Vector2 look = Vector2{ 0.0f, 0.0f };
        bool joyActive = false;
        bool lookActive = false;

        int count = GetTouchPointCount();
        for (int i = 0; i < count; i++) {
            int id = GetTouchPointId(i);
            Vector2 tp = GetTouchPosition(i);

            if (tp.x < sw * 0.5f) {
                if (joyId == -1) {
                    joyId = id;
                    joyOrigin = tp;
                }
                if (joyId == id) {
                    Vector2 d = Vector2{ tp.x - joyOrigin.x, tp.y - joyOrigin.y };
                    float len = sqrtf(d.x * d.x + d.y * d.y);
                    if (len > joyRadius) {
                        d.x *= joyRadius / len;
                        d.y *= joyRadius / len;
                    }
                    joyKnob = d;
                    joyActive = true;
                }
            } else {
                if (lookId == -1) {
                    lookId = id;
                    lookLast = tp;
                }
                if (lookId == id) {
                    look.x += tp.x - lookLast.x;
                    look.y += tp.y - lookLast.y;
                    lookLast = tp;
                    lookActive = true;
                }
            }
        }
        if (!joyActive) {
            joyId = -1;
            joyKnob = Vector2{ 0.0f, 0.0f };
        }
        if (!lookActive) {
            lookId = -1;
        }

        if (joyActive) {
            ix += joyKnob.x / joyRadius;
            iz -= joyKnob.y / joyRadius;
        }
        float ilen = sqrtf(ix * ix + iz * iz);
        float mag = fminf(ilen, 1.0f);
        if (ilen > 1.0f) {
            ix /= ilen;
            iz /= ilen;
        }

        camYaw -= look.x * 0.005f;
        camPitch = Clampf(camPitch + look.y * 0.003f, 0.15f, 1.2f);

        float fx = -sinf(camYaw);
        float fz = -cosf(camYaw);
        float rx = cosf(camYaw);
        float rz = -sinf(camYaw);
        float dirX = fx * iz + rx * ix;
        float dirZ = fz * iz + rz * ix;

        float wantX = dirX * moveSpeed * mag;
        float wantZ = dirZ * moveSpeed * mag;
        float accel = fminf(dt * 8.0f, 1.0f);
        p.vel.x += (wantX - p.vel.x) * accel;
        p.vel.z += (wantZ - p.vel.z) * accel;
        p.pos.x += p.vel.x * dt;
        p.pos.z += p.vel.z * dt;

        float speed = sqrtf(p.vel.x * p.vel.x + p.vel.z * p.vel.z);
        if (speed > 0.2f) {
            float targetYaw = atan2f(p.vel.x, p.vel.z);
            p.yaw += WrapAngle(targetYaw - p.yaw) * fminf(dt * 10.0f, 1.0f);
        }

        p.pos.x = Clampf(p.pos.x, -60.0f, 60.0f);
        p.pos.z = Clampf(p.pos.z, -60.0f, 60.0f);

        bool moving = mag > 0.05f;
        UpdateAnim(anim, dt, Clampf(speed / moveSpeed, 0.0f, 1.0f), moving);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), switchBtn)) {
            p.female = !p.female;
        }
        if (IsKeyPressed(KEY_TAB)) {
            p.female = !p.female;
        }

        Body body = p.female ? FemaleBody() : MaleBody();
        ComputePose(pose, anim, body);

        Vector3 target = V3(p.pos.x, 1.0f, p.pos.z);
        float cp = cosf(camPitch);
        cam.position = V3(
            target.x + sinf(camYaw) * cp * camDist,
            target.y + sinf(camPitch) * camDist,
            target.z + cosf(camYaw) * cp * camDist
        );
        cam.target = target;

        BeginDrawing();
        ClearBackground(Color{ 150, 200, 240, 255 });

        BeginMode3D(cam);
        DrawPlane(V3(0.0f, -0.01f, 0.0f), Vector2{ 200.0f, 200.0f }, Color{ 88, 140, 78, 255 });
        DrawGrid(100, 2.0f);

        float blink = BlinkAmount(anim);
        if (p.female) {
            DrawFemaleCharacter(pose, p.pos, p.yaw, blink);
        } else {
            DrawMaleCharacter(pose, p.pos, p.yaw, blink);
        }
        EndMode3D();

        if (joyActive) {
            DrawCircleV(joyOrigin, joyRadius, Fade(GRAY, 0.35f));
            DrawCircleV(Vector2{ joyOrigin.x + joyKnob.x, joyOrigin.y + joyKnob.y }, 40.0f, Fade(WHITE, 0.7f));
        }

        DrawRectangleRec(switchBtn, Fade(BLACK, 0.5f));
        DrawRectangleLinesEx(switchBtn, 2.0f, WHITE);
        DrawText(p.female ? "Karakter: Wanita" : "Karakter: Pria",
                 (int)switchBtn.x + 12, (int)switchBtn.y + 16, 18, WHITE);

        DrawText("ZoneWars", 20, 20, 30, WHITE);
        DrawText("Geser kiri: jalan | Geser kanan: kamera", 20, 60, 16, WHITE);
        DrawFPS(20, sh - 30);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}