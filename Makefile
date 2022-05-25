# install prefix either /usr or /usr/local on most unix systems
PREFIX ?= /usr

APP := goniometer

# comment out to disable SDL GUI
SDL_CFLAGS := -DHAVE_SDL $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image)
SDL_LDLIBS := $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image)

# comment out to disable ALSA input
ALSA_CFLAGS := -DHAVE_ALSA $(shell pkg-config --cflags alsa)
ALSA_LDLIBS := $(shell pkg-config --libs alsa)
ALSA_OBJS   := alsa.o

# comment out to disable PORTAUDIO input
PORTAUDIO_CFLAGS := -DHAVE_PORTAUDIO $(shell pkg-config --cflags portaudio-2.0)
PORTAUDIO_LDLIBS := $(shell pkg-config --libs portaudio-2.0)
PORTAUDIO_OBJS   := portaudio.o

# comment out to disable PULSE input
PULSE_CFLAGS     := -DHAVE_PULSE $(shell pkg-config --cflags libpulse-simple)
PULSE_LDLIBS     := $(shell pkg-config --libs libpulse-simple)
PULSE_OBJS       := pulse.o

# comment out to disable SNDIO input
SNDIO_CLFAGS := -DHAVE_SNDIO $(shell pkg-config --cflags sndio)
SNDIO_LDLIBS := $(shell pkg-config --libs sndio)
SNDIO_OBJS   := sndio.o

CFLAGS  := -std=c99 -O2 -g -Wall -Wextra -Wno-unused-result -Wno-maybe-uninitialized -Wno-vla-parameter
CFLAGS  += $(SDL_CFLAGS) $(ALSA_CFLAGS) $(PORTAUDIO_CFLAGS) $(PULSE_CFLAGS) $(SNDIO_CFLAGS)
LDLIBS  += $(SDL_LDLIBS) $(ALSA_LDLIBS) $(PORTAUDIO_LDLIBS) $(PULSE_LDLIBS) $(SNDIO_LDLIBS) -lm
LDFLAGS += -Wl,--as-needed

all: $(APP)

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $+

$(APP): goniometer.o common.o fifo.o $(ALSA_OBJS) $(PORTAUDIO_OBJS) $(PULSE_OBJS) $(SNDIO_OBJS)
	$(CC) -o $@ $+ $(LDFLAGS) $(LDLIBS)

install: $(APP)
	strip $(APP)
	mkdir -p $(INSTALL_PREFIX)$(PREFIX)/bin
	install -m 0755 $(APP) $(INSTALL_PREFIX)$(PREFIX)/bin

uninstall:
	rm $(INSTALL_PREFIX)$(PREFIX)/bin/$(APP)

clean:
	rm -f *~ *.o $(APP)

pull:
	git pull origin main --rebase

push:
	git push origin HEAD:main

