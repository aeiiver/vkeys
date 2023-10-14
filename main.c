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
    SDL_Window *win;
    SDL_Renderer *rend;
} audio;

static float hs_to_freqf(Halfstep hs)
{
    return FREQ_0 * SDL_powf(2, hs / 12.f);
}

static void play_sin_wave(float amplitude, Halfstep hs, float halfsecs)
{
    float freq = hs_to_freqf(hs) / (float)audio.spec.freq;
    for (float t = 0; t < (float)audio.spec.freq * halfsecs; ++t) {
        int16_t buf = (int16_t)(amplitude * SDL_sinf(PIPI * freq * t));
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

static Halfstep lookup(SDL_Scancode code)
{
    switch (code) {
    case SDL_SCANCODE_1: return 0 + 14;
    case SDL_SCANCODE_2: return 1 + 14;
    case SDL_SCANCODE_3: return 2 + 14;
    case SDL_SCANCODE_4: return 3 + 14;
    case SDL_SCANCODE_5: return 4 + 14;
    case SDL_SCANCODE_6: return 5 + 14;
    case SDL_SCANCODE_7: return 6 + 14;
    case SDL_SCANCODE_8: return 7 + 14;
    case SDL_SCANCODE_9: return 8 + 14;
    case SDL_SCANCODE_0: return 9 + 14;

    case SDL_SCANCODE_Q: return 0 + 7;
    case SDL_SCANCODE_W: return 1 + 7;
    case SDL_SCANCODE_E: return 2 + 7;
    case SDL_SCANCODE_R: return 3 + 7;
    case SDL_SCANCODE_T: return 4 + 7;
    case SDL_SCANCODE_Y: return 5 + 7;
    case SDL_SCANCODE_U: return 6 + 7;
    case SDL_SCANCODE_I: return 7 + 7;
    case SDL_SCANCODE_O: return 8 + 7;
    case SDL_SCANCODE_P: return 9 + 7;

    case SDL_SCANCODE_A: return 0;
    case SDL_SCANCODE_S: return 1;
    case SDL_SCANCODE_D: return 2;
    case SDL_SCANCODE_F: return 3;
    case SDL_SCANCODE_G: return 4;
    case SDL_SCANCODE_H: return 5;
    case SDL_SCANCODE_J: return 6;
    case SDL_SCANCODE_K: return 7;
    case SDL_SCANCODE_L: return 8;
    case SDL_SCANCODE_SEMICOLON: return 9;

    case SDL_SCANCODE_Z: return 0 - 7;
    case SDL_SCANCODE_X: return 1 - 7;
    case SDL_SCANCODE_C: return 2 - 7;
    case SDL_SCANCODE_V: return 3 - 7;
    case SDL_SCANCODE_B: return 4 - 7;
    case SDL_SCANCODE_N: return 5 - 7;
    case SDL_SCANCODE_M: return 6 - 7;
    case SDL_SCANCODE_COMMA: return 7 - 7;
    case SDL_SCANCODE_PERIOD: return 8 - 7;
    case SDL_SCANCODE_SLASH: return 9 - 7;

    default: return INT8_MIN;
    }
}

static int play_note(Halfstep hs)
{
    float ampl = 4000;
    float wait = .2f;

    SDL_ClearQueuedAudio(audio.dev);
    play_sin_wave(ampl, Scale_at(Scale_minor, Key_C, hs), wait);

    return 0;
}

static void init()
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
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
    SDL_PauseAudioDevice(audio.dev, 0);

    if (SDL_CreateWindowAndRenderer(256, 256, 0, &audio.win, &audio.rend) == -1) {
        fprintf(stderr, "ERROR: Failed to create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_SetRenderDrawColor(audio.rend, 16, 16, 16, 255);
    SDL_RenderClear(audio.rend);
    SDL_RenderPresent(audio.rend);
}

static void clean()
{
    SDL_DestroyRenderer(audio.rend);
    SDL_DestroyWindow(audio.win);
    SDL_CloseAudioDevice(audio.dev);
    SDL_Quit();
}

static void listen()
{
    int ctrl = 0;
    for (SDL_Event evt;;) {
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {

            case SDL_QUIT:
                return;

            case SDL_KEYDOWN: {
                SDL_Keysym ks = evt.key.keysym;

                printf("sym: %s\n", SDL_GetKeyName(ks.sym));

                if (ks.sym == SDLK_RCTRL || ks.sym == SDLK_LCTRL)
                    ctrl |= ks.sym;
                if (ctrl && ks.sym == SDLK_q)
                    return;

                Halfstep hs = lookup(ks.scancode);
                if (hs == INT8_MIN)
                    break;
                play_note(hs);
            } break;

            case SDL_KEYUP:
                ctrl = 0;
                break;
            }
        }
    }
}

int main(void)
{
    init();
    listen();
    clean();
    return 0;
}
