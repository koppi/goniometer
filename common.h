#pragma once

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// number of samples to read from audio source per channel
#define BUFFER_SIZE 256

struct audio_data {
    double *in;

    int input_buffer_size;
    int buffer_size;

    int format;
    unsigned int rate;
    unsigned int channels;
    char *source;  // alsa device, fifo path or pulse source
    int im;        // input mode alsa, fifo, pulse, portaudio or sndio
    int terminate; // shared variable used to terminate audio thread
    char error_message[1024];
    int samples_counter;
    pthread_mutex_t lock;
};

void reset_output_buffers(struct audio_data *data);

int write_to_input_buffers(int16_t size, int16_t buf[size], void *data);

extern pthread_mutex_t lock;
