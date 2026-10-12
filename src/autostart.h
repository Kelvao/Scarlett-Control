#ifndef AUTOSTART_H
#define AUTOSTART_H

#include <stddef.h>

/* Resolves the standard XDG autostart desktop file path.
 * Returns 0 on success, or -1 if $HOME is unset or invalid. */
int autostart_get_path(char *buf, size_t buf_size);

/* Checks whether autostart is currently enabled.
 * Returns 1 if enabled, 0 if disabled or path unavailable. */
int autostart_is_enabled(void);

/* Enables or disables autostart by creating or deleting the .desktop file.
 * Returns 0 on success, or negative on error. */
int autostart_set_enabled(int enable);

#endif /* AUTOSTART_H */
