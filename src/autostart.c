#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "autostart.h"

int autostart_get_path(char *buf, size_t buf_size) {
    if (!buf || buf_size == 0) return -1;

    const char *config_home = getenv("XDG_CONFIG_HOME");
    if (config_home && config_home[0]) {
        snprintf(buf, buf_size, "%s/autostart/scarlett-control.desktop", config_home);
    } else {
        const char *home = getenv("HOME");
        if (!home || !home[0]) return -1;
        snprintf(buf, buf_size, "%s/.config/autostart/scarlett-control.desktop", home);
    }
    return 0;
}

int autostart_is_enabled(void) {
    char path[1024];
    if (autostart_get_path(path, sizeof(path)) != 0) return 0;
    return (access(path, F_OK) == 0);
}

static void ensure_autostart_dir(void) {
    const char *config_home = getenv("XDG_CONFIG_HOME");
    char dir[1024];
    if (config_home && config_home[0]) {
        mkdir(config_home, 0755);
        snprintf(dir, sizeof(dir), "%s/autostart", config_home);
        mkdir(dir, 0755);
    } else {
        const char *home = getenv("HOME");
        if (home && home[0]) {
            snprintf(dir, sizeof(dir), "%s/.config", home);
            mkdir(dir, 0755);
            snprintf(dir, sizeof(dir), "%s/.config/autostart", home);
            mkdir(dir, 0755);
        }
    }
}

int autostart_set_enabled(int enable) {
    char path[1024];
    if (autostart_get_path(path, sizeof(path)) != 0) return -1;

    if (!enable) {
        unlink(path);
        return 0;
    }

    ensure_autostart_dir();

    /* Determine binary or AppImage path */
    char exec_cmd[2048];
    const char *appimage = getenv("APPIMAGE");
    if (appimage && appimage[0]) {
        snprintf(exec_cmd, sizeof(exec_cmd), "\"%s\" tray", appimage);
    } else {
        char exe_buf[1024];
        ssize_t len = readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
        if (len > 0) {
            exe_buf[len] = '\0';
            snprintf(exec_cmd, sizeof(exec_cmd), "\"%s\" tray", exe_buf);
        } else {
            snprintf(exec_cmd, sizeof(exec_cmd), "scarlett-control tray");
        }
    }

    FILE *f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f,
        "[Desktop Entry]\n"
        "Name=Scarlett 2i2 Control\n"
        "Comment=Hardware control and system tray for Focusrite Scarlett 2i2\n"
        "Exec=%s\n"
        "Icon=scarlett-control\n"
        "Terminal=false\n"
        "Type=Application\n"
        "Categories=AudioVideo;Audio;Mixer;\n"
        "StartupNotify=false\n"
        "X-GNOME-Autostart-enabled=true\n",
        exec_cmd
    );
    fclose(f);
    return 0;
}

