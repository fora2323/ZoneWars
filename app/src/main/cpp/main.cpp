#include "raylib.h"
#include "character.h"
#include "female_character.h"
#include <cmath>
#include <vector>

struct Block {
    Vector3 pos;
    Vector3 size;
    Color color;
};

struct Player {
    Vector3 pos;
    float yaw;
    float phase;
    float amp;
    bool female;
};

static float Clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

int main(void)
{
    InitWindow(0, 0, "ZoneWars");
    SetTargetFPS(60);

    std::vector<Block> blocks = {
        { Vector3{ 6.0f, 1.0f, 4.0f }, Vector3{ 3.0f, 2.0f, 3.0f }, Color{ 150, 150, 160, 255 } },
        { Vector3{ -8.0f, 1.5f, -6.0f }, Vector3{ 4.0f, 3.0f, 2.0f }, Color{ 140, 110, 80, 255 } },
        { Vector3{ 12.0f, 0.5f, -10.0f }, Vector3{ 6.0f, 1.0f, 6.0f }, Color{ 120, 120, 120, 255 } },
        { Vector3{ -14.0f, 2.0f, 8.0f }, Vector3{ 2.0f, 4.0f, 6.0f }, Color{ 90, 100, 130, 255 } },
        { Vector3{ 0.0f, 0.75f, -14.0f }, Vector3{ 10.0f, 1.5f, 1.0f }, Color{ 170, 60, 60, 255 } },
        { Vector3{ -18.0f, 1.0f, -16.0f }, Vector3{ 3.0f, 2.0f, 3.0f }, Color{ 180, 160, 60, 255 } },
        { Vector3{ 18.0f, 1.25f, 12.0f }, Vector3{ 4.0f, 2.5f, 4.0f }, Color{ 60, 130, 90, 255 } },
        { Vector3{ 4.0f, 0.5f, 20.0f }, Vector3{ 8.0f, 1.0f, 2.0f }, Color{ 110, 90, 150, 255 } },
    };

    Camera3D cam = { 0 };
    cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    cam.fovy = 60.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    Player p = { Vector3{ 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f, false };

    float camYaw = 0.0f;
    float camPitch = 0.45f;
    const float camDist = 7.0f;
    const float joyRadius = 90.0f;
    const float moveSpeed = 4.0f;
    const float playerRadius = 0.35f;

    int joyId = -1;
    Vector2 joyOrigin = Vector2{ 0.0f, 0.0f };
    Vector2 joyKnob = Vector2{ 0.0f, 0.0f };
    int lookId = -1;
    Vector2 lookLast = Vector2{ 0.0f, 0.0f };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
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
        float mx = fx * iz + rx * ix;
        float mz = fz * iz + rz * ix;
        bool moving = (ix != 0.0f || iz != 0.0f);

        if (moving) {
            p.pos.x += mx * moveSpeed * dt;
            p.pos.z += mz * moveSpeed * dt;
            p.yaw = atan2f(mx, mz);
        }

        p.pos.x = Clampf(p.pos.x, -55.0f, 55.0f);
        p.pos.z = Clampf(p.pos.z, -55.0f, 55.0f);

        for (const Block& b : blocks) {
            float hx = b.size.x * 0.5f + playerRadius;
            float hz = b.size.z * 0.5f + playerRadius;
            float dx = p.pos.x - b.pos.x;
            float dz = p.pos.z - b.pos.z;
            if (fabsf(dx) < hx && fabsf(dz) < hz) {
                float px = hx - fabsf(dx);
                float pz = hz - fabsf(dz);
                if (px < pz) {
                    p.pos.x += (dx >= 0.0f ? px : -px);
                } else {
                    p.pos.z += (dz >= 0.0f ? pz : -pz);
                }
            }
        }

        float targetAmp = moving ? 1.0f : 0.0f;
        p.amp += (targetAmp - p.amp) * Clampf(dt * 8.0f, 0.0f, 1.0f);
        p.phase += dt * 9.0f;
        if (p.phase > 6.2831853f) p.phase -= 6.2831853f;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), switchBtn)) {
            p.female = !p.female;
        }
        if (IsKeyPressed(KEY_TAB)) {
            p.female = !p.female;
        }

        Vector3 target = Vector3{ p.pos.x, 1.2f, p.pos.z };
        float cp = cosf(camPitch);
        cam.position = Vector3{
            target.x + sinf(camYaw) * cp * camDist,
            target.y + sinf(camPitch) * camDist,
            target.z + cosf(camYaw) * cp * camDist
        };
        cam.target = target;

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(cam);
        DrawPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector2{ 120.0f, 120.0f }, Color{ 60, 130, 60, 255 });
        for (const Block& b : blocks) {
            DrawCube(b.pos, b.size.x, b.size.y, b.size.z, b.color);
            DrawCubeWires(b.pos, b.size.x, b.size.y, b.size.z, Fade(BLACK, 0.35f));
        }
        if (p.female) {
            DrawFemaleCharacter(p.pos, p.yaw, p.phase, p.amp);
        } else {
            DrawMaleCharacter(p.pos, p.yaw, p.phase, p.amp);
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
