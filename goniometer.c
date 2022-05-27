#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "debug.h"
#include "util.h"

#include "alsa.h"
#include "common.h"
#include "fifo.h"
#include "portaudio.h"
#include "pulse.h"
#include "sndio.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifdef __GNUC__
#undef GCC_UNUSED
#define GCC_UNUSED __attribute__((unused))
#else
#define GCC_UNUSED /* nothing */
#endif

int screen_width  = 256;
int screen_height = 256;

SDL_Window  * window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect cur_bounds;

#ifdef HAVE_ALSA
static bool is_loop_device_for_sure(const char *text) {
    const char *const LOOPBACK_DEVICE_PREFIX = "hw:Loopback,";
    return strncmp(text, LOOPBACK_DEVICE_PREFIX, strlen(LOOPBACK_DEVICE_PREFIX)) == 0;
}

static bool directory_exists(const char *path) {
    DIR *const dir = opendir(path);
    if (dir == NULL)
	return false;

    closedir(dir);
    return true;
}
#endif

#include "icons/goniometer.cdata"
void SDL_SetWindowIconMemory(SDL_Window *window) {
    SDL_RWops * z = SDL_RWFromMem(goniometer_png, sizeof(goniometer_png));
    SDL_Surface *window_icon_surface = IMG_Load_RW(z, 1);
    if (window_icon_surface == NULL) {
        fprintf(stderr, "Error: unable to load application icon: '%s'.\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_SetWindowIcon(window, window_icon_surface);
    SDL_FreeSurface(window_icon_surface);
}

int SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius) {
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                     x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                     x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                     x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                     x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}

void execute(double *in, unsigned int new_samples, struct audio_data audio, int frame_time) {
    
    //debug("new_samples = % 8d\n", new_samples);

    // do not overflow
    if (new_samples > (unsigned int)audio.input_buffer_size) {
        new_samples = (unsigned int)audio.input_buffer_size;
    }

    SDL_Rect cur_bounds;
    SDL_GetWindowPosition(window, &cur_bounds.x, &cur_bounds.y);
    SDL_GetWindowSize(window, &cur_bounds.w, &cur_bounds.h);

    screen_width = cur_bounds.w;
    screen_height = cur_bounds.h;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 64+32);
    SDL_Rect rect = { 0, 0, screen_width, screen_height };
    SDL_RenderFillRect(renderer, &rect);

    if (new_samples > 0) {
        for (unsigned int i = 0; i < new_samples - 1; i++) {
#define DX (screen_width  / 2)
#define DY (screen_height / 2)
#define deg2rad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
            
            float theta = deg2rad(180);
            
            float scale = 0.01; //XXX
            float px1 = DX + in[i * 2    ] * scale;
            float py1 = DY + in[i * 2 + 1] * scale;
            //float px2 = DX + in[(i+1) * 2    ] * scale;
            //float py2 = DY + in[(i+1) * 2 + 1] * scale;
            
            float ox = DX;
            float oy = DY;
            
            float pnx1 = cos(theta) * (px1-ox) - sin(theta) * (py1-oy) + ox;
            float pny1 = sin(theta) * (px1-ox) + cos(theta) * (py1-oy) + oy;
            //float pnx2 = cos(theta) * (px2-ox) - sin(theta) * (py2-oy) + ox;
            //float pny2 = sin(theta) * (px2-ox) + cos(theta) * (py2-oy) + oy;
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
            //SDL_RenderDrawLine(renderer, pnx1, pny1, pnx2, pny2);
            
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 16);
            SDL_RenderFillCircle(renderer, pnx1, pny1, 2);
            
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 32);
            SDL_Rect rect3 = { pnx1, pny1, 1, 1 };
            SDL_RenderFillRect(renderer, &rect3);
        }
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(frame_time);
}

SDL_Rect toggle_fake_fullscreen(SDL_Rect old_bounds) {
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_BORDERLESS) {
        SDL_SetWindowBordered(window, SDL_TRUE);
        SDL_SetWindowSize(window, old_bounds.w, old_bounds.h);
        SDL_SetWindowPosition(window, old_bounds.x, old_bounds.y);
        return old_bounds;
    } else {
        SDL_Rect cur_bounds;
        SDL_GetWindowPosition(window, &cur_bounds.x, &cur_bounds.y);
        SDL_GetWindowSize(window, &cur_bounds.w, &cur_bounds.h);

        int idx = SDL_GetWindowDisplayIndex(window);
        SDL_Rect bounds;
        SDL_GetDisplayBounds(idx, &bounds);
        SDL_SetWindowBordered(window, SDL_FALSE);
        //SDL_SetWindowPosition(window, bounds.x, bounds.y);
        SDL_SetWindowSize(window, bounds.w, bounds.h);

        return cur_bounds;
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    struct audio_data audio;
    memset(&audio, 0, sizeof(audio));

    // TODO make this a parameter
    int input = INPUT_PULSE;
    //int input = INPUT_ALSA;
    char* audio_source = "auto"; // pulse
    //char* audio_source = "hw:Loopback,1"; // alsa
    audio.source = malloc(1 + strlen(audio_source));
    strcpy(audio.source, audio_source);
    
    audio.format = -1;
    audio.rate = 0;
    audio.samples_counter = 0;
    audio.channels = 2;
    
    audio.input_buffer_size = BUFFER_SIZE * audio.channels;
    audio.buffer_size = audio.input_buffer_size * 8;

    audio.in = (double *)malloc(audio.buffer_size * sizeof(double));
    memset(audio.in, 0, sizeof(int) * audio.buffer_size);
    
    audio.terminate = 0;
    
    //debug("starting audio thread\n");

    pthread_t p_thread;
	int timeout_counter = 0;

    struct timespec timeout_timer = {.tv_sec = 0, .tv_nsec = 1000000};
    int thr_id GCC_UNUSED;

    switch (input) {
#ifdef HAVE_ALSA
	case INPUT_ALSA:
        // input_alsa: wait for the input to be ready
	    if (is_loop_device_for_sure(audio.source)) {
	        if (directory_exists("/sys/")) {
	            if (!directory_exists("/sys/module/snd_aloop/")) {
	                //XXXcleanup();
                    fprintf(stderr,
                            "Linux kernel module \"snd_aloop\" does not seem to  be loaded.\n"
                            "Maybe run \"sudo modprobe snd_aloop\".\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        thr_id = pthread_create(&p_thread, NULL, input_alsa,
                                (void *)&audio); // starting alsamusic listener
        
        timeout_counter = 0;
        
	    while (audio.format == -1 || audio.rate == 0) {
	        nanosleep(&timeout_timer, NULL);
	        timeout_counter++;
	        if (timeout_counter > 2000) {
                //XXXcleanup();
                fprintf(stderr, "could not get rate and/or format, problems with audio thread? "
                        "quiting...\n");
                exit(EXIT_FAILURE);
            }
        }
        debug("got format: %d and rate %d\n", audio.format, audio.rate);
        break;
#endif
    case INPUT_FIFO:
        // starting fifomusic listener                                                                                                      
        thr_id = pthread_create(&p_thread, NULL, input_fifo, (void *)&audio);
        audio.rate = 44100;
        audio.format = 16;
        break;
#ifdef HAVE_PULSE
    case INPUT_PULSE:
        if (strcmp(audio.source, "auto") == 0) {
            getPulseDefaultSink((void *)&audio);
        }
        // starting pulsemusic listener                                                                                                     
        thr_id = pthread_create(&p_thread, NULL, input_pulse, (void *)&audio);
        audio.rate = 44100;
        break;
#endif
#ifdef HAVE_SNDIO
    case INPUT_SNDIO:
        thr_id = pthread_create(&p_thread, NULL, input_sndio, (void *)&audio);
        audio.rate = 44100;
        break;
#endif
#ifdef HAVE_PORTAUDIO
    case INPUT_PORTAUDIO:
        thr_id = pthread_create(&p_thread, NULL, input_portaudio, (void *)&audio);
        audio.rate = 44100;
        break;
#endif
    default:
        exit(EXIT_FAILURE); // Can't happen.
    }

    //debug("audio format: %d rate: %d channels: %d\n", audio.format, audio.rate, audio.channels);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        debug("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("goniometer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    //SDL_SetWindowResizable(window, true);
    SDL_SetWindowAlwaysOnTop(window, true);
    SDL_SetWindowIconMemory(window);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    float framerate = 60.0;
    int frame_time_msec = (1 / (float)framerate) * 1000;

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }

            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_F11) {
                cur_bounds = toggle_fake_fullscreen(cur_bounds);
            }
        }

        // process: execute
        pthread_mutex_lock(&audio.lock);
        execute(audio.in, audio.samples_counter, audio, frame_time_msec);
        if (audio.samples_counter > 0) {
            audio.samples_counter = 0;
        }
        pthread_mutex_unlock(&audio.lock);
        
        // checking if audio thread has exited unexpectedly
        if (audio.terminate == 1) {
            //cleanup();
            fprintf(stderr, "Audio thread exited unexpectedly. %s\n", audio.error_message);
            exit(EXIT_FAILURE);
        }
    }

    audio.terminate = 1;
    pthread_join(p_thread, NULL);
    
    free(audio.source);
    free(audio.in);
  
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
