#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_ERROR_LEN 1024

#ifdef HAVE_PORTAUDIO
#define HAS_PORTAUDIO true
#else
#define HAS_PORTAUDIO false
#endif

#ifdef HAVE_ALSA
#define HAS_ALSA true
#else
#define HAS_ALSA false
#endif

#ifdef HAVE_PULSE
#define HAS_PULSE true
#else
#define HAS_PULSE false
#endif

#ifdef HAVE_SNDIO
#define HAS_SNDIO true
#else
#define HAS_SNDIO false
#endif

// These are in order of least-favourable to most-favourable choices, in case
// multiple are supported and configured.
enum input_method {
    INPUT_FIFO,
    INPUT_PORTAUDIO,
    INPUT_ALSA,
    INPUT_PULSE,
    INPUT_SNDIO,
    INPUT_MAX
};

enum mono_option { LEFT, RIGHT, AVERAGE };
enum data_format { FORMAT_ASCII = 0, FORMAT_BINARY = 1, FORMAT_NTK3000 = 2 };

struct error_s {
    char message[MAX_ERROR_LEN];
    int length;
};
