#ifndef CLI_PARSE_H
#define CLI_PARSE_H

#include "scarlett_alsa.h"

/* Parses boolean strings: "1", "on", "true", "yes" -> 1
 *                         "0", "off", "false", "no" -> 0
 *                         Other -> -1 */
int parse_bool_val(const char *str);

/* Parses monitor mode strings: "off", "0" -> SCARLETT_MONITOR_OFF (0)
 *                              "mono", "1" -> SCARLETT_MONITOR_MONO (1)
 *                              "stereo", "2" -> SCARLETT_MONITOR_STEREO (2)
 *                              Other -> -1 */
int parse_monitor_mode(const char *str);

#endif /* CLI_PARSE_H */

