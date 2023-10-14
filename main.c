#include "SDL2/SDL.h"

#define ARR_LEN(xs) (sizeof(xs) / sizeof(*xs))
#define ARR_AT(xs, i) (xs)[MOD(i, (signed)ARR_LEN(xs))]
#define MOD(a, b) (((a) % (b) + (b)) % (b))

#define FREQ_0 440
#define PI 3.141592653589793238462643383279502884197169f
#define PIPI (PI * 2)

#define Key_A 0u
#define Key_As 1u
#define Key_B 2u
#define Key_C 3u
#define Key_Cs 4u
#define Key_D 5u
#define Key_Ds 6u
#define Key_E 7u
#define Key_F 8u
#define Key_Fs 9u
#define Key_G 10u
#define Key_Gs 11u

typedef int8_t Halfstep;

static struct {
    SDL_AudioSpec spec;
    SDL_AudioDeviceID dev;
} audio;

static float hs_to_freqf(Halfstep hs)
{
    return FREQ_0 * SDL_powf(2, hs / 12.f);
}

static void play_sin_wave(float amplitude, Halfstep hs, uint32_t halfsecs)
{
    float freq = hs_to_freqf(hs) / (float)audio.spec.freq;
    for (uint32_t t = 0; t < (uint32_t)audio.spec.freq * halfsecs; ++t) {
        int16_t buf = (int16_t)(amplitude * SDL_sinf(PIPI * freq * (float)t));
        SDL_QueueAudio(audio.dev, &buf, sizeof(buf));
    }
}

float dBFS(float x, float x_max);

float dBFS(float x, float x_max)
{
    return 20 * SDL_logf(x / x_max);
}

/*****************************************************************************/

const Halfstep Scale_major[] = { 0, 2, 4, 5, 7, 9, 11 };
const Halfstep Scale_minor[] = { 0, 2, 3, 5, 7, 8, 10 };
const Halfstep Scale_penta_major[] = { 0, 2, 4, 7, 9 };
const Halfstep Scale_penta_minor[] = { 0, 2, 3, 7, 8 };

#define Scale_at(scale, key, note)                               \
    (Halfstep)(                                                  \
        ARR_AT(scale, note)                                      \
        + (signed)(key)                                          \
        + ((note) >= 0                                           \
               ? 12 * ((note) / (signed)ARR_LEN(scale))          \
               : 12 * ((note) / (signed)ARR_LEN(scale))          \
                   - (MOD(note, (signed)ARR_LEN(scale)) == 0 ? 0 \
                                                             : 12)))

/*****************************************************************************/

static void init()
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_zero(audio.spec);
    audio.spec.freq = 48000;
    audio.spec.format = AUDIO_S16SYS;
    audio.spec.channels = 2;
    audio.spec.samples = 2048;
    audio.spec.callback = nullptr;

    audio.dev = SDL_OpenAudioDevice(nullptr, 0, &audio.spec, nullptr, 0);
    if (audio.dev == 0) {
        fprintf(stderr, "ERROR: Failed to open audio device: %s\n", SDL_GetError());
        exit(1);
    }
}

static void clean()
{
    SDL_CloseAudioDevice(audio.dev);
    SDL_Quit();
}

int main(void)
{
    init();

    float ampl = 4000;

#if 0
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -14), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -13), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -12), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -11), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -10), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -9), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -8), 2);

    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -7), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -6), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -5), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -4), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -3), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -2), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, -1), 2);

    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 0), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 1), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 2), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 3), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 4), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 5), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 6), 2);

    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 7), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 8), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 9), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 10), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 11), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 12), 1);
    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 13), 2);

    play_sin_wave(ampl, Scale_at(Scale_major, Key_A, 14), 2);
#endif

    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 0), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 1), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 2), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 3), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 4), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 5), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 4), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 3), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 2), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 1), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_major, Key_C, 0), 1);

    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 0), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 1), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 2), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 3), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 4), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 5), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 4), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 3), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 2), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 1), 1);
    play_sin_wave(ampl, Scale_at(Scale_penta_minor, Key_C, 0), 1);

    SDL_PauseAudioDevice(audio.dev, 0);
    while (SDL_GetQueuedAudioSize(audio.dev) > 0) {
        SDL_Delay(2000);
    }

    clean();
    return 0;
}
