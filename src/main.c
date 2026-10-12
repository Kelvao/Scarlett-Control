#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>

#include "scarlett_alsa.h"
#include "tray.h"
#include "cli_parse.h"
#include "i18n.h"

static volatile int g_running = 1;

static void sigint_handler(int sig) {
    (void)sig;
    g_running = 0;
}

static void print_usage(const char *prog) {
    printf("%s: %s [%s] [%s]\n\n", _("usage_word_usage"), prog, _("usage_word_cmd"), _("usage_word_opts"));
    printf("%s\n\n", _("usage_desc"));
    printf("%s\n", _("usage_cmds_title"));
    printf("  tray                   %s\n", _("cmd_tray_desc"));
    printf("  status                 %s\n", _("cmd_status_desc"));
    printf("  set [options]          %s\n", _("cmd_set_desc"));
    printf("  toggle <item>          %s\n", _("cmd_toggle_desc"));
    printf("  watch                  %s\n\n", _("cmd_watch_desc"));
    printf("%s\n", _("usage_opts_set_title"));
    printf("  --inst1 <on|off>       %s\n", _("opt_inst1_desc"));
    printf("  --air1  <on|off>       %s\n", _("opt_air1_desc"));
    printf("  --inst2 <on|off>       %s\n", _("opt_inst2_desc"));
    printf("  --air2  <on|off>       %s\n", _("opt_air2_desc"));
    printf("  --48v   <on|off>       %s\n", _("opt_48v_desc"));
    printf("  --output <mode>        %s\n", _("opt_output_desc"));
    printf("  --monitor <mode>       %s\n\n", _("opt_monitor_desc"));
    printf("%s\n", _("usage_opts_gen_title"));
    printf("  -c, --card <id|num>    %s\n", _("opt_card_desc"));
    printf("  -l, --lang <lang>      %s\n", _("opt_lang_desc"));
    printf("  -j, --json             %s\n", _("opt_json_desc"));
    printf("  -h, --help             %s\n\n", _("opt_help_desc"));
    printf("%s\n", _("usage_examples_title"));
    printf("  %s tray\n", prog);
    printf("  %s status\n", prog);
    printf("  %s set --inst1 on --air1 on --48v on --output stereo\n", prog);
    printf("  %s toggle air1\n", prog);
    printf("  %s toggle monitor\n", prog);
}

static void print_status(const scarlett_device_t *dev, int json) {
    const scarlett_state_t *st = &dev->state;
    if (json) {
        printf("{\n");
        printf("  \"card\": %d,\n", dev->card_num);
        printf("  \"card_id\": \"%s\",\n", dev->card_id);
        printf("  \"name\": \"%s\",\n", dev->card_name);
        printf("  \"input1\": {\n");
        printf("    \"inst\": %s,\n", st->inst1 ? "true" : "false");
        printf("    \"air\": %s\n", st->air1 ? "true" : "false");
        printf("  },\n");
        printf("  \"input2\": {\n");
        printf("    \"inst\": %s,\n", st->inst2 ? "true" : "false");
        printf("    \"air\": %s\n", st->air2 ? "true" : "false");
        printf("  },\n");
        printf("  \"power_48v\": %s,\n", st->p48v ? "true" : "false");
        printf("  \"direct_monitor\": \"%s\"\n", scarlett_monitor_str(st->direct_monitor));
        printf("}\n");
        return;
    }

    printf(_("cli_header"), dev->card_name, dev->card_num, dev->card_id);
    printf("\n");
    printf("%s", _("cli_input1_sec"));
    printf("   • %-22s %s\n", _("Level:"), st->inst1 ? _("INST (Instrument)") : _("LINE (Line)"));
    printf("   • %-22s %s\n", _("Air Mode:"), scarlett_onoff_str(st->air1));
    printf("\n");
    printf("%s", _("cli_input2_sec"));
    printf("   • %-22s %s\n", _("Level:"), st->inst2 ? _("INST (Instrument)") : _("LINE (Line)"));
    printf("   • %-22s %s\n", _("Air Mode:"), scarlett_onoff_str(st->air2));
    printf("\n");
    printf("%s", _("cli_power_sec"));
    printf("   • %-22s %s\n", _("Phantom Power (+48V):"), scarlett_onoff_str(st->p48v));
    printf("\n");
    printf("%s", _("cli_output_sec"));
    printf("   • %-22s %s\n", _("Direct Monitor Mode:"), scarlett_monitor_str(st->direct_monitor));
    printf("====================================================\n");
}

static int handle_toggle_cmd(scarlett_device_t *dev, int argc, char *argv[], int arg_idx) {
    if (arg_idx + 1 >= argc) {
        fprintf(stderr, "%s", _("err_specify_toggle"));
        return 1;
    }
    const char *item = argv[arg_idx + 1];
    if (strcasecmp(item, "inst1") == 0) {
        scarlett_set_inst(dev, 1, !dev->state.inst1);
        printf(_("input1_inst_toggled"), dev->state.inst1 ? _("INST (Instrument)") : _("LINE (Line)"));
    } else if (strcasecmp(item, "air1") == 0) {
        scarlett_set_air(dev, 1, !dev->state.air1);
        printf(_("input1_air_toggled"), scarlett_onoff_str(dev->state.air1));
    } else if (strcasecmp(item, "inst2") == 0) {
        scarlett_set_inst(dev, 2, !dev->state.inst2);
        printf(_("input2_inst_toggled"), dev->state.inst2 ? _("INST (Instrument)") : _("LINE (Line)"));
    } else if (strcasecmp(item, "air2") == 0) {
        scarlett_set_air(dev, 2, !dev->state.air2);
        printf(_("input2_air_toggled"), scarlett_onoff_str(dev->state.air2));
    } else if (strcasecmp(item, "48v") == 0) {
        scarlett_set_48v(dev, !dev->state.p48v);
        printf(_("power_48v_toggled"), scarlett_onoff_str(dev->state.p48v));
    } else if (strcasecmp(item, "monitor") == 0 || strcasecmp(item, "output") == 0) {
        int next = (dev->state.direct_monitor + 1) % 3;
        scarlett_set_direct_monitor(dev, next);
        printf(_("monitor_toggled"), scarlett_monitor_str(dev->state.direct_monitor));
    } else {
        fprintf(stderr, _("err_unknown_item"), item);
        return 1;
    }
    return 0;
}

static int handle_set_cmd(scarlett_device_t *dev, int argc, char *argv[], int arg_idx) {
    static struct option long_options[] = {
        {"inst1",   required_argument, 0, 1001},
        {"air1",    required_argument, 0, 1002},
        {"inst2",   required_argument, 0, 1003},
        {"air2",    required_argument, 0, 1004},
        {"48v",     required_argument, 0, 1005},
        {"output",  required_argument, 0, 1006},
        {"monitor", required_argument, 0, 1006},
        {"card",    required_argument, 0, 'c'},
        {"lang",    required_argument, 0, 'l'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    optind = arg_idx + 1;
    int opt = 0, option_index = 0;
    int any_change = 0;

    while ((opt = getopt_long(argc, argv, "c:l:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 1001: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_inst(dev, 1, val);
                    any_change = 1;
                } else fprintf(stderr, _("Invalid value for --inst1: '%s'\n"), optarg);
                break;
            }
            case 1002: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_air(dev, 1, val);
                    any_change = 1;
                } else fprintf(stderr, _("Invalid value for --air1: '%s'\n"), optarg);
                break;
            }
            case 1003: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_inst(dev, 2, val);
                    any_change = 1;
                } else fprintf(stderr, _("Invalid value for --inst2: '%s'\n"), optarg);
                break;
            }
            case 1004: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_air(dev, 2, val);
                    any_change = 1;
                } else fprintf(stderr, _("Invalid value for --air2: '%s'\n"), optarg);
                break;
            }
            case 1005: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_48v(dev, val);
                    any_change = 1;
                } else fprintf(stderr, _("Invalid value for --48v: '%s'\n"), optarg);
                break;
            }
            case 1006: {
                int mode = parse_monitor_mode(optarg);
                if (mode >= 0) {
                    scarlett_set_direct_monitor(dev, mode);
                    any_change = 1;
                } else fprintf(stderr, _("Invalid mode for output/monitor: '%s' (use: off, mono or stereo)\n"), optarg);
                break;
            }
            case 'c':
            case 'l':
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                break;
        }
    }

    if (any_change) {
        printf("%s", _("settings_updated"));
        print_status(dev, 0);
    } else {
        printf("%s", _("no_changes"));
    }
    return 0;
}

static int handle_watch_cmd(scarlett_device_t *dev) {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    scarlett_subscribe_events(dev);
    printf("%s", _("watch_starting"));
    print_status(dev, 0);

    struct pollfd pfd = { .fd = dev->ctl_fd, .events = POLLIN };
    while (g_running) {
        int pr = poll(&pfd, 1, 500);
        if (pr > 0 && (pfd.revents & POLLIN)) {
            int changed = 0;
            scarlett_process_events(dev, NULL, &changed);
            if (changed) {
                printf("%s", _("watch_event"));
                print_status(dev, 0);
            }
        }
    }

    printf("%s", _("watch_stopping"));
    return 0;
}

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
        print_usage(argv[0]);
        return 0;
    }

    /* Open device */
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

    int exit_code = 0;

    if (strcmp(command, "status") == 0) {
        print_status(&dev, json_output);
    } else if (strcmp(command, "tray") == 0) {
        ret = tray_init_and_run(&dev);
        exit_code = (ret == 0) ? 0 : 1;
    } else if (strcmp(command, "toggle") == 0) {
        exit_code = handle_toggle_cmd(&dev, argc, argv, arg_idx);
    } else if (strcmp(command, "set") == 0) {
        exit_code = handle_set_cmd(&dev, argc, argv, arg_idx);
    } else if (strcmp(command, "watch") == 0) {
        exit_code = handle_watch_cmd(&dev);
    } else {
        fprintf(stderr, _("err_unknown_cmd"), command);
        exit_code = 1;
    }

    scarlett_close(&dev);
    return exit_code;
}
