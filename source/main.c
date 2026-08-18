// Goat Simulator (Old 3DS Homebrew Port)
// Top-down 2D chaos game built with citro2d — runs on Old 3DS / 2DS.
// Circle Pad: move | A: headbutt dash | START: quit

#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define SCREEN_W 400.0f
#define SCREEN_H 240.0f
#define MAX_OBJECTS 14
#define GAME_TIME 60.0f

typedef struct { float x, y; } Vec2;

typedef struct {
    Vec2 pos;
    Vec2 vel;
    float radius;
    u32 color;
    int shape; // 0 = circle, 1 = square
} Entity;

static Entity goat;
static Entity objects[MAX_OBJECTS];
static Vec2 goatFacing = { 0.0f, -1.0f };
static int score = 0;
static int combo = 1;
static int comboTimer = 0;
static float timeLeft = GAME_TIME;
static int gameOver = 0;
static bool dashing = false;
static int dashTimer = 0;

static C2D_TextBuf g_buf;

static float frand(float lo, float hi) {
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

static void resetObject(Entity *e) {
    e->pos.x = frand(24.0f, SCREEN_W - 24.0f);
    e->pos.y = frand(40.0f, SCREEN_H - 24.0f);
    e->vel.x = 0.0f;
    e->vel.y = 0.0f;
    e->radius = frand(8.0f, 16.0f);
    e->shape = rand() % 2;
    u8 r = (u8)frand(80, 255);
    u8 g = (u8)frand(80, 255);
    u8 b = (u8)frand(80, 255);
    e->color = C2D_Color32(r, g, b, 255);
}

static void resetGame(void) {
    goat.pos.x = SCREEN_W / 2.0f;
    goat.pos.y = SCREEN_H / 2.0f;
    goat.vel.x = 0.0f;
    goat.vel.y = 0.0f;
    goat.radius = 14.0f;
    goat.color = C2D_Color32(240, 240, 240, 255);

    for (int i = 0; i < MAX_OBJECTS; i++) resetObject(&objects[i]);

    score = 0;
    combo = 1;
    comboTimer = 0;
    timeLeft = GAME_TIME;
    gameOver = 0;
    dashing = false;
    dashTimer = 0;
}

static void clampToScreen(Entity *e) {
    if (e->pos.x < e->radius) { e->pos.x = e->radius; e->vel.x *= -0.6f; }
    if (e->pos.x > SCREEN_W - e->radius) { e->pos.x = SCREEN_W - e->radius; e->vel.x *= -0.6f; }
    if (e->pos.y < 40.0f + e->radius) { e->pos.y = 40.0f + e->radius; e->vel.y *= -0.6f; }
    if (e->pos.y > SCREEN_H - e->radius) { e->pos.y = SCREEN_H - e->radius; e->vel.y *= -0.6f; }
}

static void updatePhysics(Entity *e) {
    e->pos.x += e->vel.x * (1.0f / 60.0f);
    e->pos.y += e->vel.y * (1.0f / 60.0f);
    e->vel.x *= 0.90f;
    e->vel.y *= 0.90f;
    clampToScreen(e);
}

static void handleCollisions(void) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        Entity *o = &objects[i];
        float dx = o->pos.x - goat.pos.x;
        float dy = o->pos.y - goat.pos.y;
        float dist = sqrtf(dx * dx + dy * dy);
        float minDist = o->radius + goat.radius;

        if (dist < minDist && dist > 0.001f) {
            float nx = dx / dist, ny = dy / dist;
            float overlap = (minDist - dist);
            o->pos.x += nx * overlap;
            o->pos.y += ny * overlap;

            float goatSpeed = sqrtf(goat.vel.x * goat.vel.x + goat.vel.y * goat.vel.y);
            float power = dashing ? 6.5f : 2.0f;
            o->vel.x += nx * (goatSpeed + 40.0f) * power * 0.15f;
            o->vel.y += ny * (goatSpeed + 40.0f) * power * 0.15f;

            if (dashing) {
                score += 15 * combo;
            } else if (goatSpeed > 30.0f) {
                score += 5 * combo;
            }
            comboTimer = 90;
        }

        for (int j = i + 1; j < MAX_OBJECTS; j++) {
            Entity *p = &objects[j];
            float odx = p->pos.x - o->pos.x;
            float ody = p->pos.y - o->pos.y;
            float odist = sqrtf(odx * odx + ody * ody);
            float ominDist = o->radius + p->radius;
            if (odist < ominDist && odist > 0.001f) {
                float onx = odx / odist, ony = ody / odist;
                float ov = ominDist - odist;
                o->pos.x -= onx * ov * 0.5f;
                o->pos.y -= ony * ov * 0.5f;
                p->pos.x += onx * ov * 0.5f;
                p->pos.y += ony * ov * 0.5f;

                float tmpx = o->vel.x, tmpy = o->vel.y;
                o->vel.x = p->vel.x * 0.5f;
                o->vel.y = p->vel.y * 0.5f;
                p->vel.x = tmpx * 0.5f;
                p->vel.y = tmpy * 0.5f;
            }
        }
    }
}

static void drawEntity(Entity *e) {
    if (e->shape == 0) {
        C2D_DrawCircleSolid(e->pos.x, e->pos.y, 0.0f, e->radius, e->color);
    } else {
        C2D_DrawRectSolid(e->pos.x - e->radius, e->pos.y - e->radius, 0.0f,
                           e->radius * 2.0f, e->radius * 2.0f, e->color);
    }
}

static void drawGoat(void) {
    C2D_DrawCircleSolid(goat.pos.x, goat.pos.y, 0.0f, goat.radius, goat.color);
    float hx = goat.pos.x + goatFacing.x * (goat.radius + 6.0f);
    float hy = goat.pos.y + goatFacing.y * (goat.radius + 6.0f);
    u32 hornColor = dashing ? C2D_Color32(255, 80, 40, 255) : C2D_Color32(60, 60, 60, 255);
    C2D_DrawCircleSolid(hx, hy, 0.0f, 5.0f, hornColor);
}

int main(int argc, char **argv) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    g_buf = C2D_TextBufNew(4096);

    srand((unsigned int)svcGetSystemTick());
    resetGame();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) break;

        if (gameOver) {
            if (kDown & KEY_A) resetGame();
        } else {
            circlePosition cpos;
            hidCircleRead(&cpos);
            float cx = (float)cpos.dx / 156.0f;
            float cy = (float)cpos.dy / 156.0f;
            if (cx > 1.0f) cx = 1.0f; if (cx < -1.0f) cx = -1.0f;
            if (cy > 1.0f) cy = 1.0f; if (cy < -1.0f) cy = -1.0f;

            float mag = sqrtf(cx * cx + cy * cy);
            if (mag > 0.15f) {
                goatFacing.x = cx;
                goatFacing.y = -cy;
                goat.vel.x += cx * 14.0f;
                goat.vel.y += -cy * 14.0f;
            }

            if ((kDown & KEY_A) && !dashing) {
                dashing = true;
                dashTimer = 12;
                goat.vel.x += goatFacing.x * 220.0f;
                goat.vel.y += goatFacing.y * 220.0f;
            }
            if (dashing) {
                dashTimer--;
                if (dashTimer <= 0) dashing = false;
            }

            float maxSpeed = dashing ? 420.0f : 200.0f;
            float speed = sqrtf(goat.vel.x * goat.vel.x + goat.vel.y * goat.vel.y);
            if (speed > maxSpeed) {
                goat.vel.x = goat.vel.x / speed * maxSpeed;
                goat.vel.y = goat.vel.y / speed * maxSpeed;
            }

            updatePhysics(&goat);
            for (int i = 0; i < MAX_OBJECTS; i++) updatePhysics(&objects[i]);
            handleCollisions();

            if (comboTimer > 0) {
                comboTimer--;
                combo = 1 + (score / 300);
                if (combo > 5) combo = 5;
            } else {
                combo = 1;
            }

            timeLeft -= 1.0f / 60.0f;
            if (timeLeft <= 0.0f) { timeLeft = 0.0f; gameOver = 1; }
        }

        C2D_TextBufClear(g_buf);
        char scoreStr[64], timeStr[64];
        snprintf(scoreStr, sizeof(scoreStr), "Chaos Score: %d  (x%d)", score, combo);
        snprintf(timeStr, sizeof(timeStr), "Time: %d", (int)timeLeft);
        C2D_Text scoreText, timeText;
        C2D_TextParse(&scoreText, g_buf, scoreStr);
        C2D_TextParse(&timeText, g_buf, timeStr);
        C2D_TextOptimize(&scoreText);
        C2D_TextOptimize(&timeText);

        C3D_FrameBegin(C3D_FRAME_SYNC);

        // TOP SCREEN — game view
        C2D_TargetClear(top, C2D_Color32(135, 206, 235, 255));
        C2D_SceneBegin(top);
        C2D_DrawRectSolid(0.0f, SCREEN_H - 40.0f, 0.0f, SCREEN_W, 40.0f, C2D_Color32(90, 170, 60, 255));
        for (int i = 0; i < MAX_OBJECTS; i++) drawEntity(&objects[i]);
        drawGoat();
        C2D_DrawText(&scoreText, C2D_WithColor, 6.0f, 4.0f, 0.0f, 0.55f, 0.55f, C2D_Color32(0, 0, 0, 255));
        C2D_DrawText(&timeText, C2D_WithColor, 6.0f, 20.0f, 0.0f, 0.55f, 0.55f, C2D_Color32(0, 0, 0, 255));

        if (gameOver) {
            char finalStr[64];
            snprintf(finalStr, sizeof(finalStr), "TIME UP! Final Chaos Score: %d", score);
            C2D_Text finalText, pressA;
            C2D_TextParse(&finalText, g_buf, finalStr);
            C2D_TextParse(&pressA, g_buf, "Press A to headbutt into a new round");
            C2D_TextOptimize(&finalText);
            C2D_TextOptimize(&pressA);
            C2D_DrawText(&finalText, C2D_WithColor, 55.0f, 110.0f, 0.0f, 0.6f, 0.6f, C2D_Color32(200, 0, 0, 255));
            C2D_DrawText(&pressA, C2D_WithColor, 55.0f, 128.0f, 0.0f, 0.5f, 0.5f, C2D_Color32(0, 0, 0, 255));
        }

        // BOTTOM SCREEN — instructions
        C2D_TargetClear(bottom, C2D_Color32(30, 30, 40, 255));
        C2D_SceneBegin(bottom);
        C2D_Text t1, t2, t3, t4;
        C2D_TextParse(&t1, g_buf, "GOAT SIMULATOR - 3DS");
        C2D_TextParse(&t2, g_buf, "Circle Pad: Move");
        C2D_TextParse(&t3, g_buf, "A: Headbutt Dash (launch objects!)");
        C2D_TextParse(&t4, g_buf, "START: Quit");
        C2D_TextOptimize(&t1); C2D_TextOptimize(&t2);
        C2D_TextOptimize(&t3); C2D_TextOptimize(&t4);
        C2D_DrawText(&t1, C2D_WithColor, 40.0f, 30.0f, 0.0f, 0.7f, 0.7f, C2D_Color32(255, 255, 255, 255));
        C2D_DrawText(&t2, C2D_WithColor, 20.0f, 80.0f, 0.0f, 0.55f, 0.55f, C2D_Color32(200, 200, 200, 255));
        C2D_DrawText(&t3, C2D_WithColor, 20.0f, 100.0f, 0.0f, 0.55f, 0.55f, C2D_Color32(200, 200, 200, 255));
        C2D_DrawText(&t4, C2D_WithColor, 20.0f, 120.0f, 0.0f, 0.55f, 0.55f, C2D_Color32(200, 200, 200, 255));

        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(g_buf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
