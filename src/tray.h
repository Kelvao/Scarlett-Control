#ifndef TRAY_H
#define TRAY_H

#include "scarlett_alsa.h"

/* Run the system tray event loop.
 * Returns 0 on clean exit, or negative if tray couldn't be initialized. */
int tray_init_and_run(scarlett_device_t *dev);

#endif /* TRAY_H */

