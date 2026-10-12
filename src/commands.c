#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>

#include "commands.h"
#include "tray.h"
#include "cli_parse.h"
#include "i18n.h"

static volatile int g_watch_running = 1;

static void sigint_handler(int sig) {
    (void)sig;
    g_watch_running = 0;
}

void cmd_print_usage(const char *prog) {
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

void cmd_print_status(const scarlett_device_t *dev, int json) {
    if (!dev) return;
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

int cmd_status_run(const cmd_context_t *ctx) {
    if (!ctx || !ctx->dev) return 1;
    cmd_print_status(ctx->dev, ctx->json);
    return 0;
}

int cmd_tray_run(const cmd_context_t *ctx) {
    if (!ctx || !ctx->dev) return 1;
    int ret = tray_init_and_run(ctx->dev);
    return (ret == 0) ? 0 : 1;
}

int cmd_toggle_run(const cmd_context_t *ctx) {
    if (!ctx || !ctx->dev) return 1;
    if (ctx->arg_idx + 1 >= ctx->argc) {
        fprintf(stderr, "%s", _("err_specify_toggle"));
        return 1;
    }
    const char *item = ctx->argv[ctx->arg_idx + 1];
    scarlett_device_t *dev = ctx->dev;

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

int cmd_set_run(const cmd_context_t *ctx) {
    if (!ctx || !ctx->dev) return 1;

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

    optind = ctx->arg_idx + 1;
    int opt = 0, option_index = 0;
    int any_change = 0;
    scarlett_device_t *dev = ctx->dev;

    while ((opt = getopt_long(ctx->argc, ctx->argv, "c:l:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 1001: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_inst(dev, 1, val);
                    any_change = 1;
                } else fprintf(stderr, _("err_invalid_inst1"), optarg);
                break;
            }
            case 1002: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_air(dev, 1, val);
                    any_change = 1;
                } else fprintf(stderr, _("err_invalid_air1"), optarg);
                break;
            }
            case 1003: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_inst(dev, 2, val);
                    any_change = 1;
                } else fprintf(stderr, _("err_invalid_inst2"), optarg);
                break;
            }
            case 1004: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_air(dev, 2, val);
                    any_change = 1;
                } else fprintf(stderr, _("err_invalid_air2"), optarg);
                break;
            }
            case 1005: {
                int val = parse_bool_val(optarg);
                if (val >= 0) {
                    scarlett_set_48v(dev, val);
                    any_change = 1;
                } else fprintf(stderr, _("err_invalid_48v"), optarg);
                break;
            }
            case 1006: {
                int mode = parse_monitor_mode(optarg);
                if (mode >= 0) {
                    scarlett_set_direct_monitor(dev, mode);
                    any_change = 1;
                } else fprintf(stderr, _("err_invalid_monitor"), optarg);
                break;
            }
            case 'c':
            case 'l':
                break;
            case 'h':
                cmd_print_usage(ctx->argv[0]);
                return 0;
            default:
                break;
        }
    }

    if (any_change) {
        printf("%s", _("settings_updated"));
        cmd_print_status(dev, 0);
    } else {
        printf("%s", _("no_changes"));
    }
    return 0;
}

int cmd_watch_run(const cmd_context_t *ctx) {
    if (!ctx || !ctx->dev) return 1;

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    scarlett_device_t *dev = ctx->dev;
    scarlett_subscribe_events(dev);
    printf("%s", _("watch_starting"));
    cmd_print_status(dev, 0);

    struct pollfd pfd = { .fd = dev->ctl_fd, .events = POLLIN };
    while (g_watch_running) {
        int pr = poll(&pfd, 1, 500);
        if (pr > 0 && (pfd.revents & POLLIN)) {
            int changed = 0;
            scarlett_process_events(dev, NULL, &changed);
            if (changed) {
                printf("%s", _("watch_event"));
                cmd_print_status(dev, 0);
            }
        }
    }

    printf("%s", _("watch_stopping"));
    return 0;
}

/* Open-Closed Command Registry Table */
static const command_def_t g_commands[] = {
    {"status", "cmd_status_desc", cmd_status_run, 1},
    {"tray",   "cmd_tray_desc",   cmd_tray_run,   1},
    {"toggle", "cmd_toggle_desc", cmd_toggle_run, 1},
    {"set",    "cmd_set_desc",    cmd_set_run,    1},
    {"watch",  "cmd_watch_desc",  cmd_watch_run,  1},
};

int dispatch_command(const char *name, const cmd_context_t *ctx) {
    if (!name) return -1;

    size_t count = sizeof(g_commands) / sizeof(g_commands[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcmp(g_commands[i].name, name) == 0) {
            return g_commands[i].handler(ctx);
        }
    }

    return -1;
}
