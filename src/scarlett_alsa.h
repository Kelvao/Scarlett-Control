#ifndef SCARLETT_ALSA_H
#define SCARLETT_ALSA_H

#include <stdint.h>
#include <stdbool.h>
#include <sound/asound.h>

#define SCARLETT_MAX_NAME_LEN 128

typedef enum {
    SCARLETT_MONITOR_OFF = 0,
    SCARLETT_MONITOR_MONO = 1,
    SCARLETT_MONITOR_STEREO = 2
} scarlett_monitor_mode_t;

typedef struct {
    int inst1;             /* 0 = Line, 1 = Inst */
    int air1;              /* 0 = Off, 1 = On */
    int inst2;             /* 0 = Line, 1 = Inst */
    int air2;              /* 0 = Off, 1 = On */
    int p48v;              /* 0 = Off, 1 = On */
    int direct_monitor;    /* 0 = Off, 1 = Mono, 2 = Stereo */
} scarlett_state_t;

typedef struct {
    int ctl_fd;
    int card_num;
    char card_id[64];
    char card_name[SCARLETT_MAX_NAME_LEN];
    char card_longname[SCARLETT_MAX_NAME_LEN];

    /* Control numids */
    uint32_t numid_inst1;
    uint32_t numid_air1;
    uint32_t numid_inst2;
    uint32_t numid_air2;
    uint32_t numid_48v;
    uint32_t numid_direct_monitor;

    /* Current cached state */
    scarlett_state_t state;
} scarlett_device_t;

/* Initialize and open Scarlett 2i2 control device.
 * If card_arg is NULL, auto-detects Focusrite Scarlett 2i2.
 * Returns 0 on success, negative error code on failure. */
int scarlett_open(scarlett_device_t *dev, const char *card_arg);

/* Close Scarlett device */
void scarlett_close(scarlett_device_t *dev);

/* Read current hardware state into st */
int scarlett_read_state(scarlett_device_t *dev, scarlett_state_t *st);

/* Setters */
int scarlett_set_inst(scarlett_device_t *dev, int channel, int inst);
int scarlett_set_air(scarlett_device_t *dev, int channel, int air);
int scarlett_set_48v(scarlett_device_t *dev, int on);
int scarlett_set_direct_monitor(scarlett_device_t *dev, int mode);

/* Event subscription and polling */
int scarlett_subscribe_events(scarlett_device_t *dev);
int scarlett_process_events(scarlett_device_t *dev, scarlett_state_t *st, int *changed);

/* Utility string converters */
const char *scarlett_monitor_str(int mode);
const char *scarlett_onoff_str(int val);

#endif /* SCARLETT_ALSA_H */

