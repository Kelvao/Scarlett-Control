#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scarlett_alsa.h"
#include "commands.h"
#include "i18n.h"

int main(int argc, char *argv[]) {
    const char *card_arg = NULL;
    const char *lang_arg = NULL;
    int json_output = 0;
    const char *command = NULL;
    int arg_idx = 1;

    /* First pass: parse global flags (-c, --card, -l, --lang, -j, --json, -h, --help) and detect command */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) {
            if (i + 1 < argc) {
                lang_arg = argv[++i];
            }
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            json_output = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--card") == 0) {
            if (i + 1 < argc) {
                card_arg = argv[++i];
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            command = "help";
        } else if (!command && argv[i][0] != '-') {
            command = argv[i];
            arg_idx = i;
        }
    }

    /* Initialize i18n subsystem (system locale or forced via flag / env) */
    i18n_init(lang_arg);

    if (command && (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0)) {
        cmd_print_usage(argv[0]);
        return 0;
    }

    /* Open hardware device */
    scarlett_device_t dev;
    int ret = scarlett_open(&dev, card_arg);
    if (ret < 0) {
        fprintf(stderr, _("err_open_device"), strerror(-ret), -ret);
        fprintf(stderr, "%s", _("err_check_device"));
        return 1;
    }

    /* Default action if no command passed */
    if (!command) {
        const char *disp = getenv("DISPLAY");
        const char *wayland = getenv("WAYLAND_DISPLAY");
        command = (disp || wayland) ? "tray" : "status";
    }

    /* Prepare command execution context */
    cmd_context_t ctx = {
        .dev = &dev,
        .argc = argc,
        .argv = argv,
        .arg_idx = arg_idx,
        .json = json_output
    };

    /* Dispatch command using Strategy / Polymorphic Dispatch */
    int exit_code = dispatch_command(command, &ctx);
    if (exit_code == -1) {
        fprintf(stderr, _("err_unknown_cmd"), command);
        exit_code = 1;
    }

    scarlett_close(&dev);
    return exit_code;
}
