#ifndef COMMANDS_H
#define COMMANDS_H

#include "scarlett_alsa.h"

/* Execution context passed to commands */
typedef struct {
    scarlett_device_t *dev;
    int argc;
    char **argv;
    int arg_idx;
    int json;
} cmd_context_t;

/* Function pointer interface for CLI command handlers (SOLID: Open/Closed & Strategy) */
typedef int (*cmd_handler_fn)(const cmd_context_t *ctx);

typedef struct {
    const char *name;
    const char *description_key;
    cmd_handler_fn handler;
    int requires_device;
} command_def_t;

/* Lookup and execute a command by name.
 * Returns the command exit code, or -1 if the command was not found. */
int dispatch_command(const char *name, const cmd_context_t *ctx);

/* Command Handlers */
int cmd_status_run(const cmd_context_t *ctx);
int cmd_tray_run(const cmd_context_t *ctx);
int cmd_toggle_run(const cmd_context_t *ctx);
int cmd_set_run(const cmd_context_t *ctx);
int cmd_watch_run(const cmd_context_t *ctx);

/* Formatter: prints status in human or JSON format */
void cmd_print_status(const scarlett_device_t *dev, int json);

/* Prints command usage help */
void cmd_print_usage(const char *prog);

#endif /* COMMANDS_H */
