#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "scarlett_alsa.h"

static int str_case_contains(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    return (strcasestr(haystack, needle) != NULL);
}

static int open_card_device(int card_idx) {
    char devpath[32];
    snprintf(devpath, sizeof(devpath), "/dev/snd/controlC%d", card_idx);
    return open(devpath, O_RDWR);
}

static int find_scarlett_card(void) {
    /* Scan control devices from 0 to 31 */
    int best_card = -1;

    for (int i = 0; i < 32; i++) {
        int fd = open_card_device(i);
        if (fd < 0) continue;

        struct snd_ctl_card_info info;
        memset(&info, 0, sizeof(info));
        if (ioctl(fd, SNDRV_CTL_IOCTL_CARD_INFO, &info) == 0) {
            if (str_case_contains((const char *)info.name, "Scarlett 2i2") ||
                str_case_contains((const char *)info.id, "Scarlett 2i2") ||
                str_case_contains((const char *)info.longname, "Scarlett 2i2")) {
                best_card = i;
                close(fd);
                break;
            } else if (str_case_contains((const char *)info.name, "Scarlett") ||
                       str_case_contains((const char *)info.longname, "Scarlett")) {
                if (best_card == -1) best_card = i;
            }
        }
        close(fd);
    }

    return best_card;
}

int scarlett_open(scarlett_device_t *dev, const char *card_arg) {
    if (!dev) return -EINVAL;
    memset(dev, 0, sizeof(*dev));
    dev->ctl_fd = -1;

    int card_idx = -1;
    if (card_arg) {
        if (card_arg[0] == '/' && access(card_arg, F_OK) == 0) {
            dev->ctl_fd = open(card_arg, O_RDWR);
            if (dev->ctl_fd < 0) return -errno;
        } else if (isdigit((unsigned char)card_arg[0])) {
            card_idx = atoi(card_arg);
            dev->ctl_fd = open_card_device(card_idx);
            if (dev->ctl_fd < 0) return -errno;
        } else {
            /* Look for card by ID/name */
            for (int i = 0; i < 32; i++) {
                int fd = open_card_device(i);
                if (fd < 0) continue;
                struct snd_ctl_card_info info;
                memset(&info, 0, sizeof(info));
                if (ioctl(fd, SNDRV_CTL_IOCTL_CARD_INFO, &info) == 0) {
                    if (strcasecmp((const char *)info.id, card_arg) == 0 ||
                        str_case_contains((const char *)info.name, card_arg)) {
                        dev->ctl_fd = fd;
                        card_idx = i;
                        break;
                    }
                }
                close(fd);
            }
            if (dev->ctl_fd < 0) return -ENODEV;
        }
    } else {
        card_idx = find_scarlett_card();
        if (card_idx < 0) return -ENODEV;
        dev->ctl_fd = open_card_device(card_idx);
        if (dev->ctl_fd < 0) return -errno;
    }

    dev->card_num = card_idx;

    /* Get Card Info */
    struct snd_ctl_card_info info;
    memset(&info, 0, sizeof(info));
    if (ioctl(dev->ctl_fd, SNDRV_CTL_IOCTL_CARD_INFO, &info) == 0) {
        dev->card_num = info.card;
        strncpy(dev->card_id, (const char *)info.id, sizeof(dev->card_id) - 1);
        strncpy(dev->card_name, (const char *)info.name, sizeof(dev->card_name) - 1);
        strncpy(dev->card_longname, (const char *)info.longname, sizeof(dev->card_longname) - 1);
    }

    /* Enumerate controls to match numids */
    struct snd_ctl_elem_list list;
    memset(&list, 0, sizeof(list));
    if (ioctl(dev->ctl_fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) < 0) {
        close(dev->ctl_fd);
        dev->ctl_fd = -1;
        return -errno;
    }

    if (list.count > 0) {
        list.space = list.count;
        list.pids = calloc(list.count, sizeof(struct snd_ctl_elem_id));
        if (!list.pids) {
            close(dev->ctl_fd);
            dev->ctl_fd = -1;
            return -ENOMEM;
        }

        if (ioctl(dev->ctl_fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) == 0) {
            for (unsigned int i = 0; i < list.used; i++) {
                const char *name = (const char *)list.pids[i].name;
                uint32_t numid = list.pids[i].numid;

                /* Line In 1 Level */
                if (str_case_contains(name, "Level") &&
                    (str_case_contains(name, "Line In 1") || str_case_contains(name, "Input 1") || str_case_contains(name, " 1 "))) {
                    dev->numid_inst1 = numid;
                }
                /* Line In 2 Level */
                else if (str_case_contains(name, "Level") &&
                    (str_case_contains(name, "Line In 2") || str_case_contains(name, "Input 2") || str_case_contains(name, " 2 "))) {
                    dev->numid_inst2 = numid;
                }
                /* Line In 1 Air */
                else if (str_case_contains(name, "Air") &&
                    (str_case_contains(name, "Line In 1") || str_case_contains(name, "Input 1") || str_case_contains(name, " 1 "))) {
                    dev->numid_air1 = numid;
                }
                /* Line In 2 Air */
                else if (str_case_contains(name, "Air") &&
                    (str_case_contains(name, "Line In 2") || str_case_contains(name, "Input 2") || str_case_contains(name, " 2 "))) {
                    dev->numid_air2 = numid;
                }
                /* Phantom Power (+48V) */
                else if ((str_case_contains(name, "Phantom Power") || str_case_contains(name, "48V")) &&
                         !str_case_contains(name, "Persistence")) {
                    dev->numid_48v = numid;
                }
                /* Direct Monitor */
                else if (str_case_contains(name, "Direct Monitor")) {
                    dev->numid_direct_monitor = numid;
                }
            }
        }
        free(list.pids);
    }

    /* Read initial hardware state */
    return scarlett_read_state(dev, &dev->state);
}

void scarlett_close(scarlett_device_t *dev) {
    if (!dev) return;
    if (dev->ctl_fd >= 0) {
        close(dev->ctl_fd);
        dev->ctl_fd = -1;
    }
}

static int read_ctl_enum(int fd, uint32_t numid, int *val) {
    if (!numid || fd < 0) return -EINVAL;
    struct snd_ctl_elem_value elem;
    memset(&elem, 0, sizeof(elem));
    elem.id.numid = numid;
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &elem) < 0) return -errno;
    if (val) *val = (int)elem.value.enumerated.item[0];
    return 0;
}

static int write_ctl_enum(int fd, uint32_t numid, int val) {
    if (!numid || fd < 0) return -EINVAL;
    struct snd_ctl_elem_value elem;
    memset(&elem, 0, sizeof(elem));
    elem.id.numid = numid;
    elem.value.enumerated.item[0] = (unsigned int)val;
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &elem) < 0) return -errno;
    return 0;
}

static int read_ctl_bool(int fd, uint32_t numid, int *val) {
    if (!numid || fd < 0) return -EINVAL;
    struct snd_ctl_elem_value elem;
    memset(&elem, 0, sizeof(elem));
    elem.id.numid = numid;
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &elem) < 0) return -errno;
    if (val) *val = (int)elem.value.integer.value[0];
    return 0;
}

static int write_ctl_bool(int fd, uint32_t numid, int val) {
    if (!numid || fd < 0) return -EINVAL;
    struct snd_ctl_elem_value elem;
    memset(&elem, 0, sizeof(elem));
    elem.id.numid = numid;
    elem.value.integer.value[0] = val ? 1 : 0;
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &elem) < 0) return -errno;
    return 0;
}

int scarlett_read_state(scarlett_device_t *dev, scarlett_state_t *st) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;

    scarlett_state_t cur;
    memset(&cur, 0, sizeof(cur));
    int reads_attempted = 0;
    int reads_succeeded = 0;
    int last_err = 0;

    #define DO_CTL_READ(call) do { \
        reads_attempted++; \
        int r = (call); \
        if (r == 0) reads_succeeded++; \
        else last_err = r; \
    } while(0)

    if (dev->numid_inst1) DO_CTL_READ(read_ctl_enum(dev->ctl_fd, dev->numid_inst1, &cur.inst1));
    if (dev->numid_air1)  DO_CTL_READ(read_ctl_bool(dev->ctl_fd, dev->numid_air1, &cur.air1));
    if (dev->numid_inst2) DO_CTL_READ(read_ctl_enum(dev->ctl_fd, dev->numid_inst2, &cur.inst2));
    if (dev->numid_air2)  DO_CTL_READ(read_ctl_bool(dev->ctl_fd, dev->numid_air2, &cur.air2));
    if (dev->numid_48v)   DO_CTL_READ(read_ctl_bool(dev->ctl_fd, dev->numid_48v, &cur.p48v));
    if (dev->numid_direct_monitor) DO_CTL_READ(read_ctl_enum(dev->ctl_fd, dev->numid_direct_monitor, &cur.direct_monitor));

    #undef DO_CTL_READ

    if (reads_attempted > 0 && reads_succeeded == 0) {
        return last_err < 0 ? last_err : -EIO;
    }

    dev->state = cur;
    if (st) *st = cur;
    return 0;
}

int scarlett_set_inst(scarlett_device_t *dev, int channel, int inst) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;
    uint32_t numid = (channel == 1) ? dev->numid_inst1 : dev->numid_inst2;
    if (!numid) return -ENOTSUP;
    int ret = write_ctl_enum(dev->ctl_fd, numid, inst ? 1 : 0);
    if (ret == 0) {
        if (channel == 1) dev->state.inst1 = inst ? 1 : 0;
        else dev->state.inst2 = inst ? 1 : 0;
    }
    return ret;
}

int scarlett_set_air(scarlett_device_t *dev, int channel, int air) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;
    uint32_t numid = (channel == 1) ? dev->numid_air1 : dev->numid_air2;
    if (!numid) return -ENOTSUP;
    int ret = write_ctl_bool(dev->ctl_fd, numid, air ? 1 : 0);
    if (ret == 0) {
        if (channel == 1) dev->state.air1 = air ? 1 : 0;
        else dev->state.air2 = air ? 1 : 0;
    }
    return ret;
}

int scarlett_set_48v(scarlett_device_t *dev, int on) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;
    if (!dev->numid_48v) return -ENOTSUP;
    int ret = write_ctl_bool(dev->ctl_fd, dev->numid_48v, on ? 1 : 0);
    if (ret == 0) dev->state.p48v = on ? 1 : 0;
    return ret;
}

int scarlett_set_direct_monitor(scarlett_device_t *dev, int mode) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;
    if (!dev->numid_direct_monitor) return -ENOTSUP;
    if (mode < 0 || mode > 2) return -EINVAL;
    int ret = write_ctl_enum(dev->ctl_fd, dev->numid_direct_monitor, mode);
    if (ret == 0) dev->state.direct_monitor = mode;
    return ret;
}

int scarlett_subscribe_events(scarlett_device_t *dev) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;
    int enable = 1;
    return ioctl(dev->ctl_fd, SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS, &enable);
}

int scarlett_process_events(scarlett_device_t *dev, scarlett_state_t *st, int *changed) {
    if (!dev || dev->ctl_fd < 0) return -EINVAL;
    if (changed) *changed = 0;

    int did_change = 0;
    struct snd_ctl_event ev;
    
    /* Set non-blocking temporarily or read available events */
    int flags = fcntl(dev->ctl_fd, F_GETFL, 0);
    fcntl(dev->ctl_fd, F_SETFL, flags | O_NONBLOCK);

    ssize_t rd = 0;
    while ((rd = read(dev->ctl_fd, &ev, sizeof(ev))) > 0) {
        if (ev.type == SNDRV_CTL_EVENT_ELEM) {
            uint32_t nid = ev.data.elem.id.numid;
            if (nid == dev->numid_inst1 || nid == dev->numid_air1 ||
                nid == dev->numid_inst2 || nid == dev->numid_air2 ||
                nid == dev->numid_48v || nid == dev->numid_direct_monitor) {
                did_change = 1;
            }
        }
    }

    int saved_errno = errno;
    fcntl(dev->ctl_fd, F_SETFL, flags);

    if (rd < 0 && saved_errno != EAGAIN && saved_errno != EWOULDBLOCK) {
        return -saved_errno;
    }

    if (did_change) {
        scarlett_read_state(dev, &dev->state);
        if (changed) *changed = 1;
    }
    if (st) *st = dev->state;
    return 0;
}

const char *scarlett_monitor_str(int mode) {
    switch (mode) {
        case SCARLETT_MONITOR_OFF:    return "Off";
        case SCARLETT_MONITOR_MONO:   return "Mono";
        case SCARLETT_MONITOR_STEREO: return "Stereo";
        default:                      return "Unknown";
    }
}

const char *scarlett_onoff_str(int val) {
    return val ? "ON" : "OFF";
}
