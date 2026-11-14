#include "raylib.h"
#include "character.h"
#include "female_character.h"
#include <cmath>

struct Player {
    Vector3 pos;
    Vector3 vel;
    float yaw;
    bool female;
    bool grounded;
    bool sprinting;
    bool crouching;
    bool sliding;
    float slideTimer;
    Vector3 slideDir;
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

// Cek apakah ada sentuhan baru di dalam tombol UI
static bool IsTouchButtonPressed(Rectangle rect)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect)) {
        return true;
    }
    return false;
}

static bool IsPointInAnyUI(Vector2 pt, const Rectangle* rects, int count)
{
    for (int i = 0; i < count; i++) {
        if (CheckCollisionPointRec(pt, rects[i])) return true;
    }
    return false;
}

// Render Map Datar Taktis Hemat Memori (0 Tekstur, Ramah RAM 2GB)
static void DrawFlatArenaMap(Vector3 playerPos)
{
    const Color floorBase = Color{ 58, 66, 76, 255 };
    const Color tileAlt   = Color{ 66, 75, 86, 255 };
    const Color lineCol   = Color{ 88, 100, 114, 255 };
    const Color borderCol = Color{ 190, 55, 65, 255 };

    // Lantai utama datar penuh
    DrawPlane(V3(0.0f, 0.0f, 0.0f), Vector2{ 140.0f, 140.0f }, floorBase);

    // Ubin taktis datar di sekitar kamera/pemain supaya tidak berat merender ribuan poligon jauh
    const float tileSize = 4.0f;
    int centerX = (int)floorf(playerPos.x / tileSize);
    int centerZ = (int)floorf(playerPos.z / tileSize);
    const int radius = 7;

    for (int gz = centerZ - radius; gz <= centerZ + radius; gz++) {
        for (int gx = centerX - radius; gx <= centerX + radius; gx++) {
            if (((gx + gz) & 1) == 0) {
                float wx = gx * tileSize + tileSize * 0.5f;
                float wz = gz * tileSize + tileSize * 0.5f;
                if (fabsf(wx) < 64.0f && fabsf(wz) < 64.0f) {
                    DrawPlane(V3(wx, 0.001f, wz), Vector2{ tileSize * 0.96f, tileSize * 0.96f }, tileAlt);
                }
            }
        }
    }

    // Garis penanda tengah arena datar
    DrawPlane(V3(0.0f, 0.002f, 0.0f), Vector2{ 120.0f, 0.18f }, lineCol);
    DrawPlane(V3(0.0f, 0.002f, 0.0f), Vector2{ 0.18f, 120.0f }, lineCol);

    // Garis batas zona luar
    DrawPlane(V3(0.0f, 0.002f,  60.0f), Vector2{ 120.0f, 0.4f }, borderCol);
    DrawPlane(V3(0.0f, 0.002f, -60.0f), Vector2{ 120.0f, 0.4f }, borderCol);
    DrawPlane(V3( 60.0f, 0.002f, 0.0f), Vector2{ 0.4f, 120.0f }, borderCol);
    DrawPlane(V3(-60.0f, 0.002f, 0.0f), Vector2{ 0.4f, 120.0f }, borderCol);
}

static void DrawUIButton(Rectangle r, const char* label, bool active, Color activeColor)
{
    Color bg = active ? Fade(activeColor, 0.80f) : Fade(Color{ 20, 24, 32, 255 }, 0.65f);
    DrawRectangleRounded(r, 0.35f, 6, bg);
    DrawRectangleRoundedLines(r, 0.35f, 6, 2.0f, active ? WHITE : Fade(WHITE, 0.65f));
    int tw = MeasureText(label, 18);
    DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + (r.height - 18) * 0.5f), 18, WHITE);
}

int main(void)
{
    InitWindow(0, 0, "ZoneWars");
    SetTargetFPS(60);

    Camera3D cam = { 0 };
    cam.up = V3(0.0f, 1.0f, 0.0f);
    cam.fovy = 52.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    Player p = {
        V3(0.0f, 0.0f, 0.0f),
        V3(0.0f, 0.0f, 0.0f),
        0.0f,
        true,   // Default karakter wanita
        true,   // grounded
        false,  // sprinting
        false,  // crouching
        false,  // sliding
        0.0f,
        V3(0.0f, 0.0f, 1.0f)
    };

    AnimState anim;
    InitAnim(anim);
    Pose pose;

    float camYaw = 0.0f;
    float camPitch = 0.32f;
    const float camDist = 3.8f;
    const float joyRadius = 85.0f;

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

        // Tata letak tombol layar sentuh
        Rectangle switchBtn = Rectangle{ (float)sw - 195.0f, 20.0f, 175.0f, 48.0f };
        Rectangle sprintBtn = Rectangle{ (float)sw - 135.0f, (float)sh - 290.0f, 115.0f, 54.0f };
        Rectangle jumpBtn   = Rectangle{ (float)sw - 135.0f, (float)sh - 220.0f, 115.0f, 58.0f };
        Rectangle slideBtn  = Rectangle{ (float)sw - 265.0f, (float)sh - 145.0f, 115.0f, 54.0f };
        Rectangle crouchBtn = Rectangle{ (float)sw - 135.0f, (float)sh - 145.0f, 115.0f, 54.0f };
        Rectangle uiRects[5] = { switchBtn, sprintBtn, jumpBtn, slideBtn, crouchBtn };

        // Input Keyboard (untuk tes PC jika diperlukan)
        float ix = 0.0f;
        float iz = 0.0f;
        if (IsKeyDown(KEY_W)) iz += 1.0f;
        if (IsKeyDown(KEY_S)) iz -= 1.0f;
        if (IsKeyDown(KEY_D)) ix += 1.0f;
        if (IsKeyDown(KEY_A)) ix -= 1.0f;

        // Input Multi-Touch Android (Kiri: Analog Jalan, Kanan: Kamera + Tombol Aksi)
        Vector2 look = Vector2{ 0.0f, 0.0f };
        bool joyActive = false;
        bool lookActive = false;

        int count = GetTouchPointCount();
        for (int i = 0; i < count; i++) {
            int id = GetTouchPointId(i);
            Vector2 tp = GetTouchPosition(i);

            if (tp.x < sw * 0.48f) {
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
                // Abaikan geseran kamera jika jari memulai sentuhan di atas tombol UI
                if (lookId == -1 && !IsPointInAnyUI(tp, uiRects, 5)) {
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

        // Otomatis lari jika analog didorong penuh (> 88%) atau tombol LARI aktif
        bool fullPushSprint = (mag > 0.88f && !p.crouching);
        bool activeSprint = (p.sprinting || fullPushSprint || IsKeyDown(KEY_LEFT_SHIFT)) && !p.crouching;

        // Aksi Tombol UI
        if (IsTouchButtonPressed(switchBtn) || IsKeyPressed(KEY_TAB)) {
            p.female = !p.female;
        }
        if (IsTouchButtonPressed(sprintBtn)) {
            p.sprinting = !p.sprinting;
            if (p.sprinting) p.crouching = false;
        }
        if (IsTouchButtonPressed(crouchBtn) || IsKeyPressed(KEY_C)) {
            p.crouching = !p.crouching;
            if (p.crouching) p.sprinting = false;
        }

        // Aksi Slide (Meluncur)
        if ((IsTouchButtonPressed(slideBtn) || IsKeyPressed(KEY_LEFT_CONTROL)) && p.grounded && !p.sliding) {
            p.sliding = true;
            p.crouching = false;
            p.slideTimer = 0.72f;
            p.slideDir = V3(sinf(p.yaw), 0.0f, cosf(p.yaw));
            p.vel.x = p.slideDir.x * 8.2f;
            p.vel.z = p.slideDir.z * 8.2f;
        }

        // Aksi Lompat
        if ((IsTouchButtonPressed(jumpBtn) || IsKeyPressed(KEY_SPACE)) && p.grounded) {
            p.grounded = false;
            p.sliding = false;
            p.crouching = false;
            p.vel.y = 6.2f;
        }

        // Rotasi Kamera
        camYaw -= look.x * 0.005f;
        camPitch = Clampf(camPitch + look.y * 0.0035f, 0.10f, 1.15f);

        // Vektor Arah Relatif Kamera
        float fx = -sinf(camYaw);
        float fz = -cosf(camYaw);
        float rx = cosf(camYaw);
        float rz = -sinf(camYaw);
        float dirX = fx * iz + rx * ix;
        float dirZ = fz * iz + rz * ix;

        // Kecepatan berdasarkan state (Jongkok, Jalan, Lari, atau Slide)
        float maxSpeed = 3.2f;
        if (activeSprint) maxSpeed = 6.0f;
        else if (p.crouching) maxSpeed = 1.85f;

        if (p.sliding) {
            p.slideTimer -= dt;
            float friction = fmaxf(0.0f, 1.0f - dt * 2.8f);
            p.vel.x *= friction;
            p.vel.z *= friction;
            if (p.slideTimer <= 0.0f) {
                p.sliding = false;
                p.crouching = true; // Selesai slide otomatis masuk posisi jongkok siap
            }
        } else {
            float wantX = dirX * maxSpeed * mag;
            float wantZ = dirZ * maxSpeed * mag;
            float accel = fminf(dt * (p.grounded ? 10.0f : 4.0f), 1.0f);
            p.vel.x += (wantX - p.vel.x) * accel;
            p.vel.z += (wantZ - p.vel.z) * accel;
        }

        // Fisika Gravitasi & Lantai Datar (Y = 0.0f)
        if (!p.grounded) {
            p.vel.y -= 15.5f * dt;
            p.pos.y += p.vel.y * dt;
            if (p.pos.y <= 0.0f) {
                p.pos.y = 0.0f;
                p.vel.y = 0.0f;
                p.grounded = true;
                anim.landSquash = 1.0f; // Efek tekuk lutut saat mendarat
            }
        }

        p.pos.x = Clampf(p.pos.x + p.vel.x * dt, -58.0f, 58.0f);
        p.pos.z = Clampf(p.pos.z + p.vel.z * dt, -58.0f, 58.0f);

        float horizSpeed = sqrtf(p.vel.x * p.vel.x + p.vel.z * p.vel.z);
        float prevYaw = p.yaw;
        if (horizSpeed > 0.15f && !p.sliding) {
            float targetYaw = atan2f(p.vel.x, p.vel.z);
            p.yaw += WrapAngle(targetYaw - p.yaw) * fminf(dt * 12.0f, 1.0f);
        }
        float yawRate = WrapAngle(p.yaw - prevYaw) / fmaxf(dt, 0.001f);

        bool moving = mag > 0.05f && horizSpeed > 0.15f;
        float speedNorm = Clampf(horizSpeed / 6.0f, 0.0f, 1.0f);

        UpdateAnim(anim, dt, speedNorm, moving, activeSprint,
                   p.crouching, p.sliding, p.grounded, p.vel.y, yawRate);

        Body body = p.female ? FemaleBody() : MaleBody();
        ComputePose(pose, anim, body);

        // Kamera mengikuti ketinggian pinggul/dada secara mulus saat lompat atau slide
        float camTargetY = p.pos.y + 1.05f - 0.22f * anim.crouchBlend - 0.35f * anim.slideBlend;
        Vector3 target = V3(p.pos.x, camTargetY, p.pos.z);
        float cp = cosf(camPitch);
        cam.position = V3(
            target.x + sinf(camYaw) * cp * camDist,
            target.y + sinf(camPitch) * camDist,
            target.z + cosf(camYaw) * cp * camDist
        );
        cam.target = target;

        BeginDrawing();
        ClearBackground(Color{ 135, 175, 215, 255 });

        BeginMode3D(cam);
        DrawFlatArenaMap(p.pos);

        float blink = BlinkAmount(anim);
        if (p.female) {
            DrawFemaleCharacter(pose, p.pos, p.yaw, blink);
        } else {
            DrawMaleCharacter(pose, p.pos, p.yaw, blink);
        }
        EndMode3D();

        // Render Virtual Analog Kiri
        if (joyActive) {
            DrawCircleV(joyOrigin, joyRadius, Fade(BLACK, 0.35f));
            DrawCircleLines((int)joyOrigin.x, (int)joyOrigin.y, joyRadius, Fade(WHITE, 0.5f));
            DrawCircleV(Vector2{ joyOrigin.x + joyKnob.x, joyOrigin.y + joyKnob.y }, 36.0f,
                        activeSprint ? Fade(Color{ 230, 65, 75, 255 }, 0.85f) : Fade(WHITE, 0.8f));
        }

        // Render Tombol Aksi Kanan
        DrawUIButton(switchBtn, p.female ? "Model: Wanita" : "Model: Pria", true, Color{ 138, 20, 34, 255 });
        DrawUIButton(sprintBtn, "LARI", activeSprint, Color{ 210, 85, 35, 255 });
        DrawUIButton(jumpBtn,   "LOMPAT", !p.grounded, Color{ 45, 135, 210, 255 });
        DrawUIButton(slideBtn,  "SLIDE", p.sliding, Color{ 165, 45, 185, 255 });
        DrawUIButton(crouchBtn, "JONGKOK", p.crouching, Color{ 45, 165, 105, 255 });

        DrawText("ZoneWars 3D", 24, 20, 28, WHITE);
        DrawFPS(24, sh - 32);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}