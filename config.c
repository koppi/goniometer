#include "config.h"
#include "util.h"

#include <ctype.h>
#include <iniparser.h>
#include <math.h>

#ifdef HAVE_SNDIO
#include <sndio.h>
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <sys/stat.h>

enum input_method default_methods[] = {
    INPUT_FIFO,
    INPUT_PORTAUDIO,
    INPUT_ALSA,
    INPUT_PULSE,
};

const char *input_method_names[] = {
    "fifo", "portaudio", "alsa", "pulse", "sndio",
};

const bool has_input_method[] = {
    true, /** Always have at least FIFO input. */
    HAS_PORTAUDIO, HAS_ALSA, HAS_PULSE, HAS_SNDIO,
};

enum input_method input_method_by_name(const char *str) {
    for (int i = 0; i < INPUT_MAX; i++) {
        if (!strcmp(str, input_method_names[i])) {
            return (enum input_method)i;
        }
    }

    return INPUT_MAX;
}

void write_errorf(void *err, const char *fmt, ...) {
    struct error_s *error = (struct error_s *)err;
    va_list args;
    va_start(args, fmt);
    error->length +=
        vsnprintf((char *)error->message + error->length, MAX_ERROR_LEN - error->length, fmt, args);
    va_end(args);
}
