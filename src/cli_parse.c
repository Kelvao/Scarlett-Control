#define _GNU_SOURCE
#include <string.h>
#include <strings.h>
#include "cli_parse.h"

int parse_bool_val(const char *str) {
    if (!str) return -1;
    if (strcasecmp(str, "1") == 0 || strcasecmp(str, "on") == 0 ||
        strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0) return 1;
    if (strcasecmp(str, "0") == 0 || strcasecmp(str, "off") == 0 ||
        strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0) return 0;
    return -1;
}

int parse_monitor_mode(const char *str) {
    if (!str) return -1;
    if (strcasecmp(str, "off") == 0 || strcmp(str, "0") == 0) return SCARLETT_MONITOR_OFF;
    if (strcasecmp(str, "mono") == 0 || strcmp(str, "1") == 0) return SCARLETT_MONITOR_MONO;
    if (strcasecmp(str, "stereo") == 0 || strcmp(str, "2") == 0) return SCARLETT_MONITOR_STEREO;
    return -1;
}

