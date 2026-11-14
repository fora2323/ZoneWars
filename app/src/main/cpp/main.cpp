#include "raylib.h"
#include "rlgl.h"
#include "math_anim_helper.h"
#include "render_helper.h"
#include "female_character.h"

struct Player {
    Vector3 pos;
    Vector3 vel;
    float yaw;
    bool grounded;
    bool sprinting;
    bool crouching;
    bool sliding;
    bool focusMode;
    int shoulderMode;   // 0 = Bahu Kanan, 1 = Bahu Kiri, 2 = Lihat Depan (Wajah), 3 = Tengah
    float slideTimer;
    Vector3 slideDir;
};

static bool IsPointInAnyUI(Vector2 pt, const Rectangle* rects, int count)
{
    for (int i = 0; i < count; i++) {
        if (CheckCollisionPointRec(pt, rects[i])) return true;
    }
    return false;
}

// =========================================================
// MAP DATAR ANTI-BLING (ZERO Z-FIGHTING & SANGAT CEPAT 60 FPS)
// Semua ubin digambar berdampingan pada Y = 0.0f dalam 1 kali rlBegin
// tanpa ada poligon yang saling tumpang-tindih!
// =========================================================
static void DrawFlatArenaMap(Vector3 playerPos)
{
    const Color tileDark  = Color{ 46, 52, 62, 255 };
    const Color tileLight = Color{ 55, 62, 74, 255 };
    const Color borderCol = Color{ 185, 45, 58, 255 };

    const float tileSize = 4.0f;
    const int gridHalf = 16; // Arena 128m x 128m

    rlDisableBackfaceCulling();
    rlBegin(RL_TRIANGLES);
    for (int gz = -gridHalf; gz < gridHalf; gz++) {
        for (int gx = -gridHalf; gx < gridHalf; gx++) {
            Color c = (((gx + gz) & 1) == 0) ? tileLight : tileDark;
            float x0 = gx * tileSize;
            float x1 = (gx + 1) * tileSize;
            float z0 = gz * tileSize;
            float z1 = (gz + 1) * tileSize;

            rlColor4ub(c.r, c.g, c.b, 255);
            rlVertex3f(x0, 0.0f, z0);
            rlVertex3f(x0, 0.0f, z1);
            rlVertex3f(x1, 0.0f, z1);

            rlVertex3f(x0, 0.0f, z0);
            rlVertex3f(x1, 0.0f, z1);
            rlVertex3f(x1, 0.0f, z0);
        }
    }
    rlEnd();

    // Tembok pembatas rendah di pinggir arena (Tinggi 0.25m -> bebas kedip)
    DrawCube(V3(0.0f, 0.12f,  60.0f), 120.0f, 0.24f, 0.4f, borderCol);
    DrawCube(V3(0.0f, 0.12f, -60.0f), 120.0f, 0.24f, 0.4f, borderCol);
    DrawCube(V3( 60.0f, 0.12f, 0.0f), 0.4f, 0.24f, 120.0f, borderCol);
    DrawCube(V3(-60.0f, 0.12f, 0.0f), 0.4f, 0.24f, 120.0f, borderCol);
}

static void DrawUIButton(Rectangle r, const char* label, bool active, Color activeColor)
{
    Color bg = active ? Fade(activeColor, 0.85f) : Fade(Color{ 18, 22, 30, 255 }, 0.68f);
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 2.0f, active ? WHITE : Fade(WHITE, 0.65f));
    int tw = MeasureText(label, 17);
    DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + (r.height - 17) * 0.5f), 17, WHITE);
}

int main(void)
{
    InitWindow(0, 0, "ZoneWars");
    SetTargetFPS(60);

    Camera3D cam = { 0 };
    cam.up = V3(0.0f, 1.0f, 0.0f);
    cam.fovy = 48.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    Player p = {
        V3(0.0f, 0.0f, 0.0f),
        V3(0.0f, 0.0f, 0.0f),
        0.0f,
        true,   // grounded
        false,  // sprinting
        false,  // crouching
        false,  // sliding
        false,  // focusMode
        0,      // shoulderMode: 0 = Bahu Kanan, 1 = Bahu Kiri, 2 = Lihat Depan, 3 = Tengah
        0.0f,
        V3(0.0f, 0.0f, 1.0f)
    };

    AnimState anim;
    InitAnim(anim);
    FemalePose pose;

    MultiTouchState touchState;
    InitMultiTouch(touchState);

    float camYaw = 0.0f;
    float camPitch = 0.16f;
    float curCamDist = 2.15f;
    float curSideOffset = 0.36f;
    float curTargetY = 1.42f;
    const float joyRadius = 88.0f;

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

        Rectangle shoulderBtn = Rectangle{ (float)sw - 215.0f, 18.0f, 195.0f, 44.0f };
        Rectangle focusBtn    = Rectangle{ (float)sw - 265.0f, (float)sh - 220.0f, 118.0f, 56.0f };
        Rectangle sprintBtn   = Rectangle{ (float)sw - 135.0f, (float)sh - 294.0f, 118.0f, 56.0f };
        Rectangle jumpBtn     = Rectangle{ (float)sw - 135.0f, (float)sh - 220.0f, 118.0f, 56.0f };
        Rectangle slideBtn    = Rectangle{ (float)sw - 265.0f, (float)sh - 146.0f, 118.0f, 56.0f };
        Rectangle crouchBtn   = Rectangle{ (float)sw - 135.0f, (float)sh - 146.0f, 118.0f, 56.0f };
        Rectangle uiRects[6]  = { shoulderBtn, focusBtn, sprintBtn, jumpBtn, slideBtn, crouchBtn };

        float ix = 0.0f;
        float iz = 0.0f;
        if (IsKeyDown(KEY_W)) iz += 1.0f;
        if (IsKeyDown(KEY_S)) iz -= 1.0f;
        if (IsKeyDown(KEY_D)) ix += 1.0f;
        if (IsKeyDown(KEY_A)) ix -= 1.0f;

        Vector2 look = Vector2{ 0.0f, 0.0f };
        bool joyActive = false;
        bool lookActive = false;

        int count = GetTouchPointCount();
        for (int i = 0; i < count; i++) {
            int id = GetTouchPointId(i);
            Vector2 tp = GetTouchPosition(i);

            if (joyId == id || (joyId == -1 && tp.x < sw * 0.45f)) {
                if (joyId == -1) {
                    joyId = id;
                    joyOrigin = tp;
                }
                Vector2 d = Vector2{ tp.x - joyOrigin.x, tp.y - joyOrigin.y };
                float len = sqrtf(d.x * d.x + d.y * d.y);
                if (len > joyRadius) {
                    d.x *= joyRadius / len;
                    d.y *= joyRadius / len;
                }
                joyKnob = d;
                joyActive = true;
            } else if (lookId == id || (lookId == -1 && tp.x >= sw * 0.45f && !IsPointInAnyUI(tp, uiRects, 6))) {
                if (lookId == -1) {
                    lookId = id;
                    lookLast = tp;
                }
                look.x += tp.x - lookLast.x;
                look.y += tp.y - lookLast.y;
                lookLast = tp;
                lookActive = true;
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

        float sens = p.focusMode ? 0.0032f : 0.0048f;
        camYaw -= look.x * sens;
        camPitch = Clampf(camPitch + look.y * (sens * 0.75f), -0.30f, 0.85f);

        float fx = -sinf(camYaw);
        float fz = -cosf(camYaw);
        float rx = cosf(camYaw);
        float rz = -sinf(camYaw);
        float dirX = fx * iz + rx * ix;
        float dirZ = fz * iz + rz * ix;

        bool autoSprint = (mag > 0.72f && !p.crouching && !p.focusMode);
        bool activeSprint = (p.sprinting || autoSprint || IsKeyDown(KEY_LEFT_SHIFT)) && !p.crouching && !p.focusMode;

        if (IsButtonJustTapped(touchState, shoulderBtn) || IsKeyPressed(KEY_V)) {
            p.shoulderMode = (p.shoulderMode + 1) % 4;
        }
        if (IsButtonJustTapped(touchState, focusBtn) || IsKeyPressed(KEY_F)) {
            p.focusMode = !p.focusMode;
            if (p.focusMode) p.sprinting = false;
        }
        if (IsButtonJustTapped(touchState, sprintBtn)) {
            p.sprinting = !p.sprinting;
            if (p.sprinting) {
                p.crouching = false;
                p.focusMode = false;
            }
        }

        bool tapCrouch = IsButtonJustTapped(touchState, crouchBtn) || IsKeyPressed(KEY_C);
        bool tapSlide  = IsButtonJustTapped(touchState, slideBtn)  || IsKeyPressed(KEY_LEFT_CONTROL);

        if ((tapSlide || (tapCrouch && activeSprint && mag > 0.2f)) && p.grounded && !p.sliding) {
            p.sliding = true;
            p.crouching = false;
            p.focusMode = false;
            p.slideTimer = 0.72f;
            if (mag > 0.15f) {
                p.slideDir = VNorm(V3(dirX, 0.0f, dirZ));
                p.yaw = atan2f(p.slideDir.x, p.slideDir.z);
            } else {
                p.slideDir = V3(sinf(p.yaw), 0.0f, cosf(p.yaw));
            }
            p.vel.x = p.slideDir.x * 9.0f;
            p.vel.z = p.slideDir.z * 9.0f;
        } else if (tapCrouch && !p.sliding) {
            p.crouching = !p.crouching;
            if (p.crouching) p.sprinting = false;
        }

        if ((IsButtonJustTapped(touchState, jumpBtn) || IsKeyPressed(KEY_SPACE)) && p.grounded) {
            p.grounded = false;
            p.crouching = false;
            p.vel.y = 6.5f;
            if (p.sliding) {
                p.sliding = false;
                p.vel.x = p.slideDir.x * 7.8f;
                p.vel.z = p.slideDir.z * 7.8f;
            } else if (activeSprint && mag > 0.2f) {
                p.vel.x = dirX * 6.8f;
                p.vel.z = dirZ * 6.8f;
            }
        }

        float maxSpeed = 3.1f;
        if (activeSprint) maxSpeed = 6.4f;
        else if (p.crouching || p.focusMode) maxSpeed = 1.95f;

        if (p.sliding) {
            p.slideTimer -= dt;
            float friction = fmaxf(0.0f, 1.0f - dt * 2.5f);
            p.vel.x *= friction;
            p.vel.z *= friction;
            if (p.slideTimer <= 0.0f) {
                p.sliding = false;
                p.crouching = (mag <= 0.65f);
            }
        } else {
            float wantX = dirX * maxSpeed * mag;
            float wantZ = dirZ * maxSpeed * mag;
            float accel = fminf(dt * (p.grounded ? 11.0f : 3.5f), 1.0f);
            p.vel.x += (wantX - p.vel.x) * accel;
            p.vel.z += (wantZ - p.vel.z) * accel;
        }

        if (!p.grounded) {
            p.vel.y -= 16.0f * dt;
            p.pos.y += p.vel.y * dt;
            if (p.pos.y <= 0.0f) {
                p.pos.y = 0.0f;
                p.vel.y = 0.0f;
                p.grounded = true;
                anim.landSquash = 1.0f;
            }
        }

        p.pos.x = Clampf(p.pos.x + p.vel.x * dt, -58.0f, 58.0f);
        p.pos.z = Clampf(p.pos.z + p.vel.z * dt, -58.0f, 58.0f);

        float horizSpeed = sqrtf(p.vel.x * p.vel.x + p.vel.z * p.vel.z);
        float prevYaw = p.yaw;

        if (p.focusMode && !p.sliding) {
            float camForwardYaw = atan2f(fx, fz);
            p.yaw += WrapAngle(camForwardYaw - p.yaw) * fminf(dt * 15.0f, 1.0f);
        } else if (horizSpeed > 0.15f && !p.sliding) {
            float targetYaw = atan2f(p.vel.x, p.vel.z);
            p.yaw += WrapAngle(targetYaw - p.yaw) * fminf(dt * 13.0f, 1.0f);
        }
        float yawRate = WrapAngle(p.yaw - prevYaw) / fmaxf(dt, 0.001f);

        bool moving = mag > 0.05f && horizSpeed > 0.15f;
        float speedNorm = Clampf(horizSpeed / 6.4f, 0.0f, 1.0f);

        UpdateAnim(anim, dt, speedNorm, moving, activeSprint,
                   p.crouching, p.sliding, p.grounded, p.vel.y, yawRate);
        ComputeFemalePose(pose, anim);

        // --- KAMERA OVER-THE-SHOULDER & MODE LIHAT DEPAN (WAJAH) ---
        float wantSide = 0.0f;
        if (p.shoulderMode == 0) wantSide = p.focusMode ? 0.40f : 0.34f;       // Bahu Kanan
        else if (p.shoulderMode == 1) wantSide = p.focusMode ? -0.40f : -0.34f; // Bahu Kiri

        float wantDist = 2.15f;
        float wantFov  = 48.0f;
        if (p.shoulderMode == 2) {
            // Mode Lihat Depan (Menyorot wajah & tubuh dari depan secara dekat)
            wantDist = 1.45f;
            wantFov  = 42.0f;
        } else if (p.focusMode) {
            wantDist = 1.40f;
            wantFov  = 40.0f;
        } else if (p.sliding) {
            wantDist = 2.55f;
            wantFov  = 56.0f;
        } else if (activeSprint && moving) {
            wantDist = 2.50f;
            wantFov  = 54.0f;
        } else if (p.shoulderMode == 3) {
            wantDist = 2.85f;
        }

        float wantTargetY = p.pos.y + (p.shoulderMode == 2 ? 1.52f : 1.44f)
                            - 0.26f * anim.crouchBlend - 0.50f * anim.slideBlend;

        float camSmooth = fminf(dt * 10.0f, 1.0f);
        curSideOffset = Lerpf(curSideOffset, wantSide, camSmooth);
        curCamDist    = Lerpf(curCamDist, wantDist, camSmooth);
        curTargetY    = Lerpf(curTargetY, wantTargetY, fminf(dt * 14.0f, 1.0f));
        cam.fovy      = Lerpf(cam.fovy, wantFov, camSmooth);

        Vector3 shoulderAnchor = V3(
            p.pos.x + rx * curSideOffset,
            curTargetY,
            p.pos.z + rz * curSideOffset
        );

        float cp = cosf(camPitch);
        float sp = sinf(camPitch);

        if (p.shoulderMode == 2) {
            // Kamera berada di depan wajah karakter menghadap langsung ke arah karakter
            float frontX = sinf(p.yaw);
            float frontZ = cosf(p.yaw);
            cam.position = V3(
                p.pos.x + frontX * curCamDist,
                curTargetY + 0.05f,
                p.pos.z + frontZ * curCamDist
            );
            cam.target = V3(p.pos.x, curTargetY, p.pos.z);
        } else {
            cam.position = V3(
                shoulderAnchor.x - fx * cp * curCamDist,
                fmaxf(0.22f, shoulderAnchor.y + sp * curCamDist),
                shoulderAnchor.z - fz * cp * curCamDist
            );
            cam.target = V3(
                shoulderAnchor.x + fx * cp * 10.0f,
                shoulderAnchor.y - sp * 10.0f,
                shoulderAnchor.z + fz * cp * 10.0f
            );
        }

        EndMultiTouchFrame(touchState);

        BeginDrawing();
        ClearBackground(Color{ 118, 152, 192, 255 });

        BeginMode3D(cam);
        DrawFlatArenaMap(p.pos);
        DrawFemaleCharacter(pose, p.pos, p.yaw, BlinkAmount(anim));
        EndMode3D();

        if (p.focusMode) {
            DrawCircle(sw / 2, sh / 2, 3.0f, Fade(WHITE, 0.85f));
            DrawCircleLines(sw / 2, sh / 2, 9.0f, Fade(WHITE, 0.55f));
        }

        if (joyActive) {
            DrawCircleV(joyOrigin, joyRadius, Fade(BLACK, 0.35f));
            DrawCircleLines((int)joyOrigin.x, (int)joyOrigin.y, joyRadius, Fade(WHITE, 0.5f));
            DrawCircleV(Vector2{ joyOrigin.x + joyKnob.x, joyOrigin.y + joyKnob.y }, 36.0f,
                        activeSprint ? Fade(Color{ 225, 55, 70, 255 }, 0.88f) : Fade(WHITE, 0.82f));
        }

        const char* shoulderLabel = "BAHU: KANAN";
        if (p.shoulderMode == 1) shoulderLabel = "BAHU: KIRI";
        else if (p.shoulderMode == 2) shoulderLabel = "KAMERA: DEPAN";
        else if (p.shoulderMode == 3) shoulderLabel = "KAMERA: TENGAH";

        DrawUIButton(shoulderBtn, shoulderLabel, p.shoulderMode == 2, Color{ 55, 95, 165, 255 });
        DrawUIButton(focusBtn,    "FOKUS",   p.focusMode,  Color{ 200, 140, 30, 255 });
        DrawUIButton(sprintBtn,   "LARI",    activeSprint, Color{ 215, 75, 40, 255 });
        DrawUIButton(jumpBtn,     "LOMPAT",  !p.grounded,  Color{ 45, 135, 210, 255 });
        DrawUIButton(slideBtn,    "SLIDE",   p.sliding,    Color{ 165, 45, 185, 255 });
        DrawUIButton(crouchBtn,   "JONGKOK", p.crouching,  Color{ 45, 165, 105, 255 });

        DrawText("ZoneWars 3D", 24, 20, 26, WHITE);
        DrawFPS(24, sh - 32);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}