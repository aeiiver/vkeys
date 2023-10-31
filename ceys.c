#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define FONT_SIZE 20
#define FONT_SPACING 2

#define VOLUME .6f
#define FREQ0 440
#define AMPLITUDE 4269
#define SAMPLES 48000
#define MAX_SAMPLES_PER_UPDATE 4096

// clang-format off
#define LIST_KEYS                                    \
    X(KEY_ONE,        (uint64_t)0x1,        0 +  +7) \
    X(KEY_TWO,        (uint64_t)0x2,        1 +  +7) \
    X(KEY_THREE,      (uint64_t)0x4,        2 +  +7) \
    X(KEY_FOUR,       (uint64_t)0x8,        3 +  +7) \
    X(KEY_FIVE,       (uint64_t)0x10,       4 +  +7) \
    X(KEY_SIX,        (uint64_t)0x20,       5 +  +7) \
    X(KEY_SEVEN,      (uint64_t)0x40,       6 +  +7) \
    X(KEY_EIGHT,      (uint64_t)0x80,       7 +  +7) \
    X(KEY_NINE,       (uint64_t)0x100,      8 +  +7) \
    X(KEY_ZERO,       (uint64_t)0x200,      9 +  +7) \
    X(KEY_Q,          (uint64_t)0x400,       0 +   0) \
    X(KEY_W,          (uint64_t)0x800,       1 +   0) \
    X(KEY_E,          (uint64_t)0x1000,      2 +   0) \
    X(KEY_R,          (uint64_t)0x2000,      3 +   0) \
    X(KEY_T,          (uint64_t)0x4000,      4 +   0) \
    X(KEY_Y,          (uint64_t)0x8000,      5 +   0) \
    X(KEY_U,          (uint64_t)0x10000,     6 +   0) \
    X(KEY_I,          (uint64_t)0x20000,     7 +   0) \
    X(KEY_O,          (uint64_t)0x40000,     8 +   0) \
    X(KEY_P,          (uint64_t)0x80000,     9 +   0) \
    X(KEY_A,          (uint64_t)0x100000,     0 +  -7) \
    X(KEY_S,          (uint64_t)0x200000,     1 +  -7) \
    X(KEY_D,          (uint64_t)0x400000,     2 +  -7) \
    X(KEY_F,          (uint64_t)0x800000,     3 +  -7) \
    X(KEY_G,          (uint64_t)0x1000000,    4 +  -7) \
    X(KEY_H,          (uint64_t)0x2000000,    5 +  -7) \
    X(KEY_J,          (uint64_t)0x4000000,    6 +  -7) \
    X(KEY_K,          (uint64_t)0x8000000,    7 +  -7) \
    X(KEY_L,          (uint64_t)0x10000000,   8 +  -7) \
    X(KEY_SEMICOLON,  (uint64_t)0x20000000,   9 +  -7) \
    X(KEY_Z,          (uint64_t)0x40000000,    0 + -14) \
    X(KEY_X,          (uint64_t)0x80000000,    1 + -14) \
    X(KEY_C,          (uint64_t)0x100000000,   2 + -14) \
    X(KEY_V,          (uint64_t)0x200000000,   3 + -14) \
    X(KEY_B,          (uint64_t)0x400000000,   4 + -14) \
    X(KEY_N,          (uint64_t)0x800000000,   5 + -14) \
    X(KEY_M,          (uint64_t)0x1000000000,  6 + -14) \
    X(KEY_COMMA,      (uint64_t)0x2000000000,  7 + -14) \
    X(KEY_PERIOD,     (uint64_t)0x4000000000,  8 + -14) \
    X(KEY_SLASH,      (uint64_t)0x8000000000,  9 + -14)
// clang-format on

#define X(key, bitmask, note) \
    const uint64_t key##_NOTE = bitmask;
LIST_KEYS
#undef X

// clang-format off
#define Key_A   0u
#define Key_As  1u
#define Key_B   2u
#define Key_C   3u
#define Key_Cs  4u
#define Key_D   5u
#define Key_Ds  6u
#define Key_E   7u
#define Key_F   8u
#define Key_Fs  9u
#define Key_G   10u
#define Key_Gs  11u
// clang-format on

uint16_t Scale_major[] = { 0, 2, 4, 5, 7, 9, 11 };
uint16_t Scale_minor[] = { 0, 2, 3, 5, 7, 8, 10 };

#define ARR_LEN(xs) (sizeof(xs) / sizeof(*xs))
#define ARR_AT(xs, i) (xs)[MOD(i, (signed)ARR_LEN(xs))]
#define MOD(a, b) (((a) % (b) + (b)) % (b))

#define Scale_at(scale, key, note)                               \
    (int8_t)(                                                    \
        ARR_AT(scale, note)                                      \
        + (signed)(key)                                          \
        + ((note) >= 0                                           \
               ? 12 * ((note) / (signed)ARR_LEN(scale))          \
               : 12 * ((note) / (signed)ARR_LEN(scale))          \
                   - (MOD(note, (signed)ARR_LEN(scale)) == 0 ? 0 \
                                                             : 12)))

uint64_t freqs = 0;
float t = 0;

static int8_t bit_to_hs(uint64_t bit)
{
    switch (bit) {

#define X(key, bitmask, note) \
    case bitmask: return note;
        LIST_KEYS
#undef X

    default: return INT8_MIN;
    }
}

static float hs_to_freqf(int8_t halfsteps)
{
    return FREQ0 * powf(2, halfsteps / 12.f);
}

static void feed_as(void *buf_, unsigned int frames)
{
    uint16_t *buf = (uint16_t *)buf_;

    for (size_t i = 0; i < frames; ++i) {
        buf[i] = 0;
        if (freqs == 0)
            continue;

        float acc = 0;
        for (size_t n = 1; n <= KEY_SLASH_NOTE; n *= 2) {
            if (!(freqs & n))
                continue;
            float freq = hs_to_freqf(Scale_at(Scale_minor, Key_C, bit_to_hs(n)));
            acc += sinf(2 * PI * freq * t);
        }
        buf[i] = (uint16_t)(AMPLITUDE * acc);
        t += 1.f / SAMPLES;
        if (t > 1)
            t -= 1;
    }
}

/*****************************************************************************/

struct {
    AudioStream as;
} app;

static inline void init(void)
{
    InitAudioDevice();
    SetMasterVolume(VOLUME);
    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

    app.as = LoadAudioStream(SAMPLES, 16, 1);
    SetAudioStreamCallback(app.as, feed_as);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ceys");
    SetTargetFPS(30);
}

static inline void uninit(void)
{
    CloseWindow();
    UnloadAudioStream(app.as);
    CloseAudioDevice();
}

static inline void listen(void)
{
    PlayAudioStream(app.as);

    Font font = GetFontDefault();
    const char text[] = "Press some keys!";
    Vector2 text_dims = MeasureTextEx(font, text, FONT_SIZE, FONT_SPACING);

    while (!WindowShouldClose()) {
        freqs = 0;

#define X(key, bitmask, note) \
    if (IsKeyDown(key))       \
        freqs |= key##_NOTE;
        LIST_KEYS
#undef X

        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawTextEx(
            font,
            text,
            (Vector2) {
                .x = (float)GetRenderWidth() / 2 - text_dims.x / 2,
                .y = (float)GetRenderHeight() / 2 - text_dims.y / 2,
            },
            FONT_SIZE,
            FONT_SPACING,
            WHITE);
        EndDrawing();
    }
}

int main(void)
{
    init();
    listen();
    uninit();
    return 0;
}
