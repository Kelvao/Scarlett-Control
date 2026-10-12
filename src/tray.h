#ifndef TRAY_H
#define TRAY_H

#include <stddef.h>
#include "scarlett_alsa.h"

/* Get the XDG autostart desktop file path.
 * Returns 0 on success, or negative if HOME is not available. */
int get_autostart_path(char *buf, size_t buf_size);

/* Run the system tray event loop.
 * Returns 0 on clean exit, or negative if tray couldn't be initialized. */
int tray_init_and_run(scarlett_device_t *dev);

#endif /* TRAY_H */

