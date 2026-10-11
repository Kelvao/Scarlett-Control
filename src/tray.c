#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

#include "tray.h"
#include "scarlett_alsa.h"
#include "i18n.h"

/* GObject / GTK / AppIndicator Types and Enums */
typedef void* GtkWidget;
typedef void* GtkMenu;
typedef void* AppIndicator;
typedef void* GtkStatusIcon;
typedef void* GConnectFlags;
typedef void* GIOChannel;

typedef enum {
    APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
    APP_INDICATOR_CATEGORY_COMMUNICATIONS,
    APP_INDICATOR_CATEGORY_SYSTEM_SERVICES,
    APP_INDICATOR_CATEGORY_HARDWARE,
    APP_INDICATOR_CATEGORY_OTHER
} AppIndicatorCategory;

typedef enum {
    APP_INDICATOR_STATUS_PASSIVE,
    APP_INDICATOR_STATUS_ACTIVE,
    APP_INDICATOR_STATUS_ATTENTION
} AppIndicatorStatus;

/* Dynamic function pointers */
static int (*p_gtk_init_check)(int *argc, char ***argv) = NULL;
static void (*p_gtk_main)(void) = NULL;
static void (*p_gtk_main_quit)(void) = NULL;
static GtkWidget* (*p_gtk_menu_new)(void) = NULL;
static GtkWidget* (*p_gtk_menu_item_new_with_label)(const char *label) = NULL;
static GtkWidget* (*p_gtk_check_menu_item_new_with_label)(const char *label) = NULL;
static void (*p_gtk_check_menu_item_set_active)(GtkWidget *item, int is_active) = NULL;
static int (*p_gtk_check_menu_item_get_active)(GtkWidget *item) = NULL;
static GtkWidget* (*p_gtk_radio_menu_item_new_with_label)(void *group, const char *label) = NULL;
static void* (*p_gtk_radio_menu_item_get_group)(GtkWidget *item) = NULL;
static GtkWidget* (*p_gtk_separator_menu_item_new)(void) = NULL;
static void (*p_gtk_menu_shell_append)(GtkWidget *menu, GtkWidget *child) = NULL;
static void (*p_gtk_widget_show_all)(GtkWidget *widget) = NULL;
static void (*p_gtk_widget_set_sensitive)(GtkWidget *widget, int sensitive) = NULL;
static unsigned long (*p_g_signal_connect_data)(void *instance, const char *detailed_signal, void *c_handler, void *data, void *destroy_data, int connect_flags) = NULL;
static unsigned int (*p_g_timeout_add)(unsigned int interval, int (*function)(void*), void *data) = NULL;

/* AppIndicator pointers */
static AppIndicator* (*p_app_indicator_new)(const char *id, const char *icon_name, AppIndicatorCategory category) = NULL;
static void (*p_app_indicator_set_status)(AppIndicator *self, AppIndicatorStatus status) = NULL;
static void (*p_app_indicator_set_icon_full)(AppIndicator *self, const char *icon_name, const char *icon_desc) = NULL;
static void (*p_app_indicator_set_menu)(AppIndicator *self, GtkMenu *menu) = NULL;
static void (*p_app_indicator_set_title)(AppIndicator *self, const char *title) = NULL;

/* GtkStatusIcon pointers (fallback) */
static GtkStatusIcon* (*p_gtk_status_icon_new_from_icon_name)(const char *icon_name) = NULL;
static void (*p_gtk_status_icon_set_tooltip_text)(GtkStatusIcon *status_icon, const char *text) = NULL;
static void (*p_gtk_menu_popup)(GtkWidget *menu, void *parent_menu_shell, void *parent_menu_item, void *func, void *data, unsigned int button, unsigned int activate_time) = NULL;

/* State */
static scarlett_device_t *g_dev = NULL;
static int g_updating_ui = 0;

static GtkWidget *item_inst1 = NULL;
static GtkWidget *item_air1 = NULL;
static GtkWidget *item_inst2 = NULL;
static GtkWidget *item_air2 = NULL;
static GtkWidget *item_48v = NULL;

static GtkWidget *item_mon_off = NULL;
static GtkWidget *item_mon_mono = NULL;
static GtkWidget *item_mon_stereo = NULL;

static void update_ui_state(void) {
    if (!g_dev) return;
    g_updating_ui = 1;

    if (item_inst1) p_gtk_check_menu_item_set_active(item_inst1, g_dev->state.inst1);
    if (item_air1)  p_gtk_check_menu_item_set_active(item_air1, g_dev->state.air1);
    if (item_inst2) p_gtk_check_menu_item_set_active(item_inst2, g_dev->state.inst2);
    if (item_air2)  p_gtk_check_menu_item_set_active(item_air2, g_dev->state.air2);
    if (item_48v)   p_gtk_check_menu_item_set_active(item_48v, g_dev->state.p48v);

    if (g_dev->state.direct_monitor == SCARLETT_MONITOR_OFF && item_mon_off) {
        p_gtk_check_menu_item_set_active(item_mon_off, 1);
    } else if (g_dev->state.direct_monitor == SCARLETT_MONITOR_MONO && item_mon_mono) {
        p_gtk_check_menu_item_set_active(item_mon_mono, 1);
    } else if (g_dev->state.direct_monitor == SCARLETT_MONITOR_STEREO && item_mon_stereo) {
        p_gtk_check_menu_item_set_active(item_mon_stereo, 1);
    }

    g_updating_ui = 0;
}

/* Callbacks for menu toggles */
static void on_inst1_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    int active = p_gtk_check_menu_item_get_active(item);
    scarlett_set_inst(g_dev, 1, active);
}

static void on_air1_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    int active = p_gtk_check_menu_item_get_active(item);
    scarlett_set_air(g_dev, 1, active);
}

static void on_inst2_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    int active = p_gtk_check_menu_item_get_active(item);
    scarlett_set_inst(g_dev, 2, active);
}

static void on_air2_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    int active = p_gtk_check_menu_item_get_active(item);
    scarlett_set_air(g_dev, 2, active);
}

static void on_48v_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    int active = p_gtk_check_menu_item_get_active(item);
    scarlett_set_48v(g_dev, active);
}

static void on_mon_off_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    if (p_gtk_check_menu_item_get_active(item)) {
        scarlett_set_direct_monitor(g_dev, SCARLETT_MONITOR_OFF);
    }
}

static void on_mon_mono_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    if (p_gtk_check_menu_item_get_active(item)) {
        scarlett_set_direct_monitor(g_dev, SCARLETT_MONITOR_MONO);
    }
}

static void on_mon_stereo_toggled(void *item, void *user_data) {
    (void)user_data;
    if (g_updating_ui || !g_dev) return;
    if (p_gtk_check_menu_item_get_active(item)) {
        scarlett_set_direct_monitor(g_dev, SCARLETT_MONITOR_STEREO);
    }
}

static void on_quit_clicked(void *item, void *user_data) {
    (void)item;
    (void)user_data;
    p_gtk_main_quit();
}

static int g_reconnect_counter = 0;

static int on_poll_timer(void *user_data) {
    (void)user_data;
    if (!g_dev) return 1;

    if (g_dev->ctl_fd < 0) {
        /* Disconnected: retry open every ~1.5 seconds (10 ticks of 150ms) */
        if (++g_reconnect_counter >= 10) {
            g_reconnect_counter = 0;
            if (scarlett_open(g_dev, NULL) == 0) {
                printf("[scarlett-control] %s\n", _("device_reconnected"));
                scarlett_subscribe_events(g_dev);
                update_ui_state();
            }
        }
        return 1;
    }

    int changed = 0;
    int ret = scarlett_process_events(g_dev, NULL, &changed);
    if (ret < 0) {
        printf("[scarlett-control] ");
        printf(_("connection_lost"), strerror(-ret));
        printf("\n");
        scarlett_close(g_dev);
        return 1;
    }

    if (changed) {
        update_ui_state();
    }
    return 1;
}

static void on_status_icon_popup_menu(GtkStatusIcon *icon, unsigned int button, unsigned int activate_time, void *user_data) {
    (void)icon;
    GtkWidget *menu = (GtkWidget*)user_data;
    if (p_gtk_menu_popup) {
        p_gtk_menu_popup(menu, NULL, NULL, NULL, NULL, button, activate_time);
    }
}

int tray_init_and_run(scarlett_device_t *dev) {
    g_dev = dev;

    /* Load GTK3 library dynamically */
    void *gtk_lib = dlopen("libgtk-3.so.0", RTLD_LAZY | RTLD_GLOBAL);
    if (!gtk_lib) {
        fprintf(stderr, "Error: Could not load libgtk-3.so.0: %s\n", dlerror());
        return -1;
    }

    #define LOAD_SYM(lib, p, name) do { \
        *(void**)(&(p)) = dlsym(lib, #name); \
        if (!(p)) { \
            fprintf(stderr, "Error resolving symbol %s: %s\n", #name, dlerror()); \
            return -2; \
        } \
    } while(0)

    LOAD_SYM(gtk_lib, p_gtk_init_check, gtk_init_check);
    LOAD_SYM(gtk_lib, p_gtk_main, gtk_main);
    LOAD_SYM(gtk_lib, p_gtk_main_quit, gtk_main_quit);
    LOAD_SYM(gtk_lib, p_gtk_menu_new, gtk_menu_new);
    LOAD_SYM(gtk_lib, p_gtk_menu_item_new_with_label, gtk_menu_item_new_with_label);
    LOAD_SYM(gtk_lib, p_gtk_check_menu_item_new_with_label, gtk_check_menu_item_new_with_label);
    LOAD_SYM(gtk_lib, p_gtk_check_menu_item_set_active, gtk_check_menu_item_set_active);
    LOAD_SYM(gtk_lib, p_gtk_check_menu_item_get_active, gtk_check_menu_item_get_active);
    LOAD_SYM(gtk_lib, p_gtk_radio_menu_item_new_with_label, gtk_radio_menu_item_new_with_label);
    LOAD_SYM(gtk_lib, p_gtk_radio_menu_item_get_group, gtk_radio_menu_item_get_group);
    LOAD_SYM(gtk_lib, p_gtk_separator_menu_item_new, gtk_separator_menu_item_new);
    LOAD_SYM(gtk_lib, p_gtk_menu_shell_append, gtk_menu_shell_append);
    LOAD_SYM(gtk_lib, p_gtk_widget_show_all, gtk_widget_show_all);
    LOAD_SYM(gtk_lib, p_gtk_widget_set_sensitive, gtk_widget_set_sensitive);
    LOAD_SYM(gtk_lib, p_g_signal_connect_data, g_signal_connect_data);
    LOAD_SYM(gtk_lib, p_g_timeout_add, g_timeout_add);

    /* Initialize GTK without requiring command line args */
    int argc = 0;
    char **argv = NULL;
    if (!p_gtk_init_check(&argc, &argv)) {
        fprintf(stderr, "Error: gtk_init_check failed (is a display server running?)\n");
        dlclose(gtk_lib);
        return -3;
    }

    /* Try loading AppIndicator (Ayatana or Canonical) */
    void *appind_lib = dlopen("libayatana-appindicator3.so.1", RTLD_LAZY);
    if (!appind_lib) {
        appind_lib = dlopen("libappindicator3.so.1", RTLD_LAZY);
    }

    int use_appindicator = 0;
    if (appind_lib) {
        *(void**)(&p_app_indicator_new) = dlsym(appind_lib, "app_indicator_new");
        *(void**)(&p_app_indicator_set_status) = dlsym(appind_lib, "app_indicator_set_status");
        *(void**)(&p_app_indicator_set_icon_full) = dlsym(appind_lib, "app_indicator_set_icon_full");
        *(void**)(&p_app_indicator_set_menu) = dlsym(appind_lib, "app_indicator_set_menu");
        *(void**)(&p_app_indicator_set_title) = dlsym(appind_lib, "app_indicator_set_title");
        if (p_app_indicator_new && p_app_indicator_set_status && p_app_indicator_set_menu) {
            use_appindicator = 1;
        }
    }

    /* Build Menu */
    GtkWidget *menu = p_gtk_menu_new();

    /* Header */
    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "Focusrite Scarlett 2i2 (%s)", dev->card_id[0] ? dev->card_id : "USB");
    GtkWidget *hdr = p_gtk_menu_item_new_with_label(title_buf);
    p_gtk_widget_set_sensitive(hdr, 0);
    p_gtk_menu_shell_append(menu, hdr);
    p_gtk_menu_shell_append(menu, p_gtk_separator_menu_item_new());

    /* Input 1 */
    GtkWidget *sec_in1 = p_gtk_menu_item_new_with_label(_("[ Input 1 ]"));
    p_gtk_widget_set_sensitive(sec_in1, 0);
    p_gtk_menu_shell_append(menu, sec_in1);

    item_inst1 = p_gtk_check_menu_item_new_with_label(_("  Inst (Instrument)"));
    p_g_signal_connect_data(item_inst1, "toggled", on_inst1_toggled, NULL, NULL, 0);
    p_gtk_menu_shell_append(menu, item_inst1);

    item_air1 = p_gtk_check_menu_item_new_with_label(_("  Air Mode"));
    p_g_signal_connect_data(item_air1, "toggled", on_air1_toggled, NULL, NULL, 0);
    p_gtk_menu_shell_append(menu, item_air1);

    p_gtk_menu_shell_append(menu, p_gtk_separator_menu_item_new());

    /* Input 2 */
    GtkWidget *sec_in2 = p_gtk_menu_item_new_with_label(_("[ Input 2 ]"));
    p_gtk_widget_set_sensitive(sec_in2, 0);
    p_gtk_menu_shell_append(menu, sec_in2);

    item_inst2 = p_gtk_check_menu_item_new_with_label(_("  Inst (Instrument)"));
    p_g_signal_connect_data(item_inst2, "toggled", on_inst2_toggled, NULL, NULL, 0);
    p_gtk_menu_shell_append(menu, item_inst2);

    item_air2 = p_gtk_check_menu_item_new_with_label(_("  Air Mode"));
    p_g_signal_connect_data(item_air2, "toggled", on_air2_toggled, NULL, NULL, 0);
    p_gtk_menu_shell_append(menu, item_air2);

    p_gtk_menu_shell_append(menu, p_gtk_separator_menu_item_new());

    /* Phantom Power */
    GtkWidget *sec_pwr = p_gtk_menu_item_new_with_label(_("[ Power ]"));
    p_gtk_widget_set_sensitive(sec_pwr, 0);
    p_gtk_menu_shell_append(menu, sec_pwr);

    item_48v = p_gtk_check_menu_item_new_with_label(_("  +48V Phantom Power"));
    p_g_signal_connect_data(item_48v, "toggled", on_48v_toggled, NULL, NULL, 0);
    p_gtk_menu_shell_append(menu, item_48v);

    p_gtk_menu_shell_append(menu, p_gtk_separator_menu_item_new());

    /* Output / Direct Monitor */
    GtkWidget *sec_out = p_gtk_menu_item_new_with_label(_("[ Output (Direct Monitor) ]"));
    p_gtk_widget_set_sensitive(sec_out, 0);
    p_gtk_menu_shell_append(menu, sec_out);

    item_mon_off = p_gtk_radio_menu_item_new_with_label(NULL, _("  Direct Monitor: Off"));
    void *group = p_gtk_radio_menu_item_get_group(item_mon_off);
    item_mon_mono = p_gtk_radio_menu_item_new_with_label(group, _("  Direct Monitor: Mono"));
    group = p_gtk_radio_menu_item_get_group(item_mon_mono);
    item_mon_stereo = p_gtk_radio_menu_item_new_with_label(group, _("  Direct Monitor: Stereo"));

    p_g_signal_connect_data(item_mon_off, "toggled", on_mon_off_toggled, NULL, NULL, 0);
    p_g_signal_connect_data(item_mon_mono, "toggled", on_mon_mono_toggled, NULL, NULL, 0);
    p_g_signal_connect_data(item_mon_stereo, "toggled", on_mon_stereo_toggled, NULL, NULL, 0);

    p_gtk_menu_shell_append(menu, item_mon_off);
    p_gtk_menu_shell_append(menu, item_mon_mono);
    p_gtk_menu_shell_append(menu, item_mon_stereo);

    p_gtk_menu_shell_append(menu, p_gtk_separator_menu_item_new());

    /* Quit */
    GtkWidget *item_quit = p_gtk_menu_item_new_with_label(_("Quit"));
    p_g_signal_connect_data(item_quit, "activate", on_quit_clicked, NULL, NULL, 0);
    p_gtk_menu_shell_append(menu, item_quit);

    p_gtk_widget_show_all(menu);

    /* Set initial check states */
    update_ui_state();

    /* Set up tray icon */
    const char *icon_name = "scarlett-control";
    if (use_appindicator) {
        AppIndicator *indicator = p_app_indicator_new(
            "scarlett-2i2-control",
            icon_name,
            APP_INDICATOR_CATEGORY_HARDWARE
        );
        p_app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
        if (p_app_indicator_set_icon_full) {
            p_app_indicator_set_icon_full(indicator, icon_name, "Scarlett 2i2 Control");
        }
        if (p_app_indicator_set_title) {
            p_app_indicator_set_title(indicator, "Scarlett 2i2 Control");
        }
        p_app_indicator_set_menu(indicator, (GtkMenu*)menu);
    } else {
        /* Fallback to GtkStatusIcon */
        *(void**)(&p_gtk_status_icon_new_from_icon_name) = dlsym(gtk_lib, "gtk_status_icon_new_from_icon_name");
        *(void**)(&p_gtk_status_icon_set_tooltip_text) = dlsym(gtk_lib, "gtk_status_icon_set_tooltip_text");
        *(void**)(&p_gtk_menu_popup) = dlsym(gtk_lib, "gtk_menu_popup");

        if (p_gtk_status_icon_new_from_icon_name) {
            GtkStatusIcon *status_icon = p_gtk_status_icon_new_from_icon_name(icon_name);
            if (p_gtk_status_icon_set_tooltip_text) {
                p_gtk_status_icon_set_tooltip_text(status_icon, "Scarlett 2i2 Control");
            }
            p_g_signal_connect_data(status_icon, "popup-menu", on_status_icon_popup_menu, menu, NULL, 0);
            p_g_signal_connect_data(status_icon, "activate", on_status_icon_popup_menu, menu, NULL, 0);
        } else {
            fprintf(stderr, "Error: No system tray implementation available (neither AppIndicator nor GtkStatusIcon).\n");
            if (appind_lib) dlclose(appind_lib);
            if (gtk_lib) dlclose(gtk_lib);
            return -4;
        }
    }

    /* Subscribe to ALSA hardware events */
    scarlett_subscribe_events(dev);

    /* Poll ALSA events every 150ms to keep UI in real-time sync with physical knobs/buttons */
    p_g_timeout_add(150, on_poll_timer, NULL);

    printf("[scarlett-control] %s\n", _("tray_started"));
    p_gtk_main();

    if (appind_lib) dlclose(appind_lib);
    if (gtk_lib) dlclose(gtk_lib);
    return 0;
}
