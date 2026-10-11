#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>

#include "i18n.h"

#define MAX_I18N_ENTRIES 256
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 512

typedef struct {
    char key[MAX_KEY_LEN];
    char val[MAX_VAL_LEN];
} i18n_kv_t;

static i18n_lang_t g_lang = I18N_LANG_EN;
static char g_lang_code[16] = "en";
static i18n_kv_t g_entries[MAX_I18N_ENTRIES];
static size_t g_entry_count = 0;

/* Embedded fallback data so binary is 100% self-contained even without disk files */
static const struct {
    const char *key;
    const char *en;
    const char *pt;
} g_embedded_defaults[] = {
    {"app_name", "Scarlett 2i2 Control", "Controle Scarlett 2i2"},
    {"app_card_title", "Focusrite Scarlett 2i2 (%s)", "Focusrite Scarlett 2i2 (%s)"},
    {"menu_input1", "[ Input 1 ]", "[ Entrada 1 ]"},
    {"menu_input2", "[ Input 2 ]", "[ Entrada 2 ]"},
    {"menu_power", "[ Power ]", "[ Alimentação ]"},
    {"menu_output", "[ Output (Direct Monitor) ]", "[ Saída (Direct Monitor) ]"},
    {"menu_inst", "  Inst (Instrument)", "  Inst (Instrumento)"},
    {"menu_air", "  Air Mode", "  Modo Air"},
    {"menu_48v", "  +48V Phantom Power", "  +48V Phantom Power"},
    {"menu_mon_off", "  Direct Monitor: Off", "  Direct Monitor: Desligado"},
    {"menu_mon_mono", "  Direct Monitor: Mono", "  Direct Monitor: Mono"},
    {"menu_mon_stereo", "  Direct Monitor: Stereo", "  Direct Monitor: Estéreo"},
    {"menu_quit", "Quit", "Sair"},
    {"tray_started", "System tray daemon started successfully.", "Daemon da bandeja do sistema iniciado com sucesso."},
    {"device_reconnected", "Focusrite device reconnected successfully!", "Dispositivo Focusrite reconectado com sucesso!"},
    {"connection_lost", "Connection to device lost (%s). Waiting for reconnect...", "Conexão com o dispositivo perdida (%s). Aguardando reconexão..."},
    {"cli_header", "====================================================\n Focusrite %s (Card %d: %s)\n====================================================\n", "====================================================\n Focusrite %s (Placa %d: %s)\n====================================================\n"},
    {"cli_input1_sec", " [ Input 1 ]\n", " [ Entrada 1 ]\n"},
    {"cli_input2_sec", " [ Input 2 ]\n", " [ Entrada 2 ]\n"},
    {"cli_power_sec", " [ Phantom Power ]\n", " [ Alimentação Fantasma ]\n"},
    {"cli_output_sec", " [ Output / Direct Monitor ]\n", " [ Saída / Monitoramento Direto ]\n"},
    {"cli_level", "Level:", "Nível:"},
    {"cli_inst_val", "INST (Instrument)", "INST (Instrumento)"},
    {"cli_line_val", "LINE (Line)", "LINE (Linha)"},
    {"cli_air", "Air Mode:", "Modo Air:"},
    {"cli_48v", "Phantom Power (+48V):", "Alimentação Fantasma (+48V):"},
    {"cli_mon_mode", "Direct Monitor Mode:", "Modo Direct Monitor:"},
    {"settings_updated", "Settings updated successfully!\n\n", "Configurações atualizadas com sucesso!\n\n"},
    {"no_changes", "No changes requested. Use --help to see options.\n", "Nenhuma alteração solicitada. Use --help para ver as opções.\n"},
    {"input1_inst_toggled", "Input 1 Inst toggled to: %s\n", "Entrada 1 Inst alternada para: %s\n"},
    {"input1_air_toggled", "Input 1 Air toggled to: %s\n", "Entrada 1 Air alternado para: %s\n"},
    {"input2_inst_toggled", "Input 2 Inst toggled to: %s\n", "Entrada 2 Inst alternada para: %s\n"},
    {"input2_air_toggled", "Input 2 Air toggled to: %s\n", "Entrada 2 Air alternado para: %s\n"},
    {"power_48v_toggled", "Phantom Power (+48V) toggled to: %s\n", "Phantom Power (+48V) alternado para: %s\n"},
    {"monitor_toggled", "Direct Monitor toggled to: %s\n", "Direct Monitor alternado para: %s\n"},
    {"watch_starting", "Monitoring hardware events in real time (Press Ctrl+C to exit)...\n", "Monitorando eventos da placa em tempo real (Pressione Ctrl+C para sair)...\n"},
    {"watch_event", "\n[Hardware Event Detected! New state:]\n", "\n[Evento de Hardware Detectado! Novo estado:]\n"},
    {"watch_stopping", "\nStopping monitoring.\n", "\nEncerrando monitoramento.\n"},
    {"err_open_device", "Error opening Focusrite Scarlett 2i2 device: %s (%d)\n", "Erro ao abrir dispositivo Focusrite Scarlett 2i2: %s (%d)\n"},
    {"err_check_device", "Please check if interface is connected and user has permission to access /dev/snd/controlC*\n", "Verifique se a interface está conectada e se o usuário tem permissão para acessar /dev/snd/controlC*\n"},
    {"err_unknown_item", "Unknown item: %s. Use inst1, air1, inst2, air2, 48v or monitor.\n", "Item desconhecido: %s. Use inst1, air1, inst2, air2, 48v ou monitor.\n"},
    {"err_invalid_inst1", "Invalid value for --inst1: '%s'\n", "Valor inválido para --inst1: '%s'\n"},
    {"err_invalid_air1", "Invalid value for --air1: '%s'\n", "Valor inválido para --air1: '%s'\n"},
    {"err_invalid_inst2", "Invalid value for --inst2: '%s'\n", "Valor inválido para --inst2: '%s'\n"},
    {"err_invalid_air2", "Invalid value for --air2: '%s'\n", "Valor inválido para --air2: '%s'\n"},
    {"err_invalid_48v", "Invalid value for --48v: '%s'\n", "Valor inválido para --48v: '%s'\n"},
    {"err_invalid_monitor", "Invalid mode for output/monitor: '%s' (use: off, mono or stereo)\n", "Modo inválido para output/monitor: '%s' (use: off, mono ou stereo)\n"},
    {"err_specify_toggle", "Error: specify item to toggle: inst1, air1, inst2, air2, 48v, monitor\n", "Erro: especifique o item para alternar: inst1, air1, inst2, air2, 48v, monitor\n"},
    {"err_unknown_cmd", "Unknown command: '%s'. Use --help for help.\n", "Comando desconhecido: '%s'. Use --help para ajuda.\n"},
    {"usage_word_usage", "Usage", "Uso"},
    {"usage_word_cmd", "COMMAND", "COMANDO"},
    {"usage_word_opts", "OPTIONS", "OPÇÕES"},
    {"usage_desc", "Lightweight control utility for Focusrite Scarlett 2i2 on any Linux desktop.", "Controle leve para Focusrite Scarlett 2i2 em qualquer ambiente Linux."},
    {"usage_cmds_title", "COMMANDS:", "COMANDOS:"},
    {"cmd_tray_desc", "Run system tray daemon (default if GUI session is active)", "Inicia o daemon com ícone na bandeja do sistema (padrão se GUI ativa)"},
    {"cmd_status_desc", "Display current state of input and output settings", "Exibe o estado atual de todas as opções de entrada e saída"},
    {"cmd_set_desc", "Modify one or more hardware settings", "Modifica uma ou mais configurações da placa"},
    {"cmd_toggle_desc", "Toggle setting state (inst1, air1, inst2, air2, 48v, monitor)", "Alterna o estado de uma opção (inst1, air1, inst2, air2, 48v, monitor)"},
    {"cmd_watch_desc", "Monitor hardware events in real time in terminal", "Monitora eventos do hardware em tempo real no terminal"},
    {"usage_opts_set_title", "MODIFICATION OPTIONS (for 'set' command):", "OPÇÕES DE MODIFICAÇÃO (para o comando 'set'):"},
    {"opt_inst1_desc", "Set Input 1 to Instrument (on) or Line (off)", "Define se Entrada 1 é Instrumento (on) ou Linha (off)"},
    {"opt_air1_desc", "Enable or disable Air Mode on Input 1", "Ativa ou desativa modo Air na Entrada 1"},
    {"opt_inst2_desc", "Set Input 2 to Instrument (on) or Line (off)", "Define se Entrada 2 é Instrumento (on) ou Linha (off)"},
    {"opt_air2_desc", "Enable or disable Air Mode on Input 2", "Ativa ou desativa modo Air na Entrada 2"},
    {"opt_48v_desc", "Enable or disable Phantom Power (+48V) for both inputs", "Ativa ou desativa Phantom Power (+48V) para ambas as entradas"},
    {"opt_output_desc", "Set Direct Monitor / Output: off, mono, or stereo", "Define Direct Monitor / Output: off, mono ou stereo"},
    {"opt_monitor_desc", "Alias for --output (off, mono, stereo)", "Sinônimo de --output (off, mono, stereo)"},
    {"usage_opts_gen_title", "GENERAL OPTIONS:", "OPÇÕES GERAIS:"},
    {"opt_card_desc", "Specify ALSA card index, id, or device node (e.g. 2, USB)", "Especifica o número, id ou caminho da placa ALSA (ex: 2, USB)"},
    {"opt_lang_desc", "Set interface language (e.g. en, pt, es, fr, de, it, ja, zh)", "Define o idioma da interface (ex: en, pt, es, fr, de, it, ja, zh)"},
    {"opt_json_desc", "Output status in JSON format (useful for scripts)", "Saída em formato JSON (útil para scripts com 'status')"},
    {"opt_help_desc", "Display this help message", "Exibe esta ajuda"},
    {"usage_examples_title", "EXAMPLES:", "EXEMPLOS:"}
};

static void add_entry(const char *key, const char *val) {
    if (g_entry_count >= MAX_I18N_ENTRIES || !key || !val) return;
    /* Update if existing key */
    for (size_t i = 0; i < g_entry_count; i++) {
        if (strcmp(g_entries[i].key, key) == 0) {
            strncpy(g_entries[i].val, val, MAX_VAL_LEN - 1);
            g_entries[i].val[MAX_VAL_LEN - 1] = '\0';
            return;
        }
    }
    strncpy(g_entries[g_entry_count].key, key, MAX_KEY_LEN - 1);
    g_entries[g_entry_count].key[MAX_KEY_LEN - 1] = '\0';
    strncpy(g_entries[g_entry_count].val, val, MAX_VAL_LEN - 1);
    g_entries[g_entry_count].val[MAX_VAL_LEN - 1] = '\0';
    g_entry_count++;
}

/* Parse JSON string escape sequences */
static void unescape_json_str(const char *src, char *dst, size_t dst_size) {
    size_t d = 0;
    for (size_t s = 0; src[s] && d + 1 < dst_size; s++) {
        if (src[s] == '\\' && src[s + 1]) {
            s++;
            switch (src[s]) {
                case 'n': dst[d++] = '\n'; break;
                case 't': dst[d++] = '\t'; break;
                case 'r': dst[d++] = '\r'; break;
                case '\\': dst[d++] = '\\'; break;
                case '"': dst[d++] = '"'; break;
                default: dst[d++] = src[s]; break;
            }
        } else {
            dst[d++] = src[s];
        }
    }
    dst[d] = '\0';
}

/* Lightweight parser for JSON key-value translation files */
static int load_json_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size <= 0 || size > 500000) {
        fclose(fp);
        return -1;
    }

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(fp);
        return -1;
    }

    if (fread(buf, 1, size, fp) != (size_t)size) {
        free(buf);
        fclose(fp);
        return -1;
    }
    buf[size] = '\0';
    fclose(fp);

    /* Parse keys and values: "key" : "value" */
    char *p = buf;
    int entries_loaded = 0;

    while (*p) {
        /* Seek opening quote of key */
        while (*p && *p != '"') p++;
        if (!*p) break;
        p++; /* Skip opening quote */
        char *key_start = p;
        while (*p && *p != '"') {
            if (*p == '\\' && *(p + 1)) p += 2;
            else p++;
        }
        if (!*p) break;
        *p = '\0';
        char key_buf[MAX_KEY_LEN];
        unescape_json_str(key_start, key_buf, sizeof(key_buf));
        p++;

        /* Seek colon */
        while (*p && *p != ':') p++;
        if (!*p) break;
        p++;

        /* Seek opening quote of value */
        while (*p && *p != '"') p++;
        if (!*p) break;
        p++;
        char *val_start = p;
        while (*p && *p != '"') {
            if (*p == '\\' && *(p + 1)) p += 2;
            else p++;
        }
        if (!*p) break;
        *p = '\0';
        char val_buf[MAX_VAL_LEN];
        unescape_json_str(val_start, val_buf, sizeof(val_buf));
        p++;

        add_entry(key_buf, val_buf);
        entries_loaded++;
    }

    free(buf);
    return entries_loaded > 0 ? 0 : -1;
}

static void load_locale(i18n_lang_t lang, const char *code) {
    g_entry_count = 0;

    /* Populate with embedded defaults first */
    size_t embed_count = sizeof(g_embedded_defaults) / sizeof(g_embedded_defaults[0]);
    for (size_t i = 0; i < embed_count; i++) {
        const char *val = (lang == I18N_LANG_PT) ? g_embedded_defaults[i].pt : g_embedded_defaults[i].en;
        add_entry(g_embedded_defaults[i].key, val);
    }

    /* Candidate filenames */
    char filenames[5][32];
    int fn_count = 0;

    if (code && *code) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "%s.json", code);
    }

    if (lang == I18N_LANG_PT) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "pt_BR.json");
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "pt.json");
    } else if (lang == I18N_LANG_ES) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "es.json");
    } else if (lang == I18N_LANG_FR) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "fr.json");
    } else if (lang == I18N_LANG_DE) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "de.json");
    } else if (lang == I18N_LANG_IT) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "it.json");
    } else if (lang == I18N_LANG_JA) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "ja.json");
    } else if (lang == I18N_LANG_ZH) {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "zh_CN.json");
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "zh.json");
    } else {
        snprintf(filenames[fn_count++], sizeof(filenames[0]), "en.json");
    }

    /* Search paths */
    const char *paths[] = {
        "locales",
        "/usr/local/share/scarlett-control/locales",
        "/usr/share/scarlett-control/locales"
    };

    char user_path[512];
    const char *home = getenv("HOME");
    if (home) {
        snprintf(user_path, sizeof(user_path), "%s/.local/share/scarlett-control/locales", home);
    }

    for (int f = 0; f < fn_count; f++) {
        for (size_t p = 0; p < sizeof(paths) / sizeof(paths[0]); p++) {
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", paths[p], filenames[f]);
            if (access(filepath, R_OK) == 0) {
                if (load_json_file(filepath) == 0) return;
            }
        }
        if (home) {
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", user_path, filenames[f]);
            if (access(filepath, R_OK) == 0) {
                if (load_json_file(filepath) == 0) return;
            }
        }
    }
}

static void detect_lang(const char *loc, i18n_lang_t *lang, char *code, size_t code_len) {
    if (!loc || !*loc) {
        *lang = I18N_LANG_EN;
        strncpy(code, "en", code_len - 1);
        code[code_len - 1] = '\0';
        return;
    }

    if (strncasecmp(loc, "pt", 2) == 0 || strcasestr(loc, "portuguese") != NULL) {
        *lang = I18N_LANG_PT;
        strncpy(code, "pt_BR", code_len - 1);
    } else if (strncasecmp(loc, "es", 2) == 0 || strcasestr(loc, "spanish") != NULL || strcasestr(loc, "espanol") != NULL) {
        *lang = I18N_LANG_ES;
        strncpy(code, "es", code_len - 1);
    } else if (strncasecmp(loc, "fr", 2) == 0 || strcasestr(loc, "french") != NULL || strcasestr(loc, "francais") != NULL) {
        *lang = I18N_LANG_FR;
        strncpy(code, "fr", code_len - 1);
    } else if (strncasecmp(loc, "de", 2) == 0 || strcasestr(loc, "german") != NULL || strcasestr(loc, "deutsch") != NULL) {
        *lang = I18N_LANG_DE;
        strncpy(code, "de", code_len - 1);
    } else if (strncasecmp(loc, "it", 2) == 0 || strcasestr(loc, "italian") != NULL) {
        *lang = I18N_LANG_IT;
        strncpy(code, "it", code_len - 1);
    } else if (strncasecmp(loc, "ja", 2) == 0 || strcasestr(loc, "japanese") != NULL) {
        *lang = I18N_LANG_JA;
        strncpy(code, "ja", code_len - 1);
    } else if (strncasecmp(loc, "zh", 2) == 0 || strcasestr(loc, "chinese") != NULL) {
        *lang = I18N_LANG_ZH;
        strncpy(code, "zh_CN", code_len - 1);
    } else {
        *lang = I18N_LANG_EN;
        /* Copy up to 5 chars of custom code (e.g. "ru", "pl") */
        size_t len = 0;
        while (loc[len] && loc[len] != '.' && loc[len] != '@' && len < code_len - 1 && len < 8) {
            code[len] = (char)tolower((unsigned char)loc[len]);
            len++;
        }
        code[len] = '\0';
        return;
    }
    code[code_len - 1] = '\0';
}

void i18n_init(const char *lang_str) {
    const char *target = NULL;

    if (lang_str && strlen(lang_str) > 0 && strcasecmp(lang_str, "auto") != 0) {
        target = lang_str;
    } else {
        const char *env_lang = getenv("SCARLETT_LANG");
        if (env_lang && strlen(env_lang) > 0) {
            target = env_lang;
        } else {
            setlocale(LC_ALL, "");
            target = getenv("LC_ALL");
            if (!target || !*target) target = getenv("LC_MESSAGES");
            if (!target || !*target) target = getenv("LANG");
        }
    }

    detect_lang(target, &g_lang, g_lang_code, sizeof(g_lang_code));
    load_locale(g_lang, g_lang_code);
}

void i18n_set_lang(i18n_lang_t lang) {
    g_lang = lang;
    switch (lang) {
        case I18N_LANG_PT: strncpy(g_lang_code, "pt_BR", sizeof(g_lang_code) - 1); break;
        case I18N_LANG_ES: strncpy(g_lang_code, "es", sizeof(g_lang_code) - 1); break;
        case I18N_LANG_FR: strncpy(g_lang_code, "fr", sizeof(g_lang_code) - 1); break;
        case I18N_LANG_DE: strncpy(g_lang_code, "de", sizeof(g_lang_code) - 1); break;
        case I18N_LANG_IT: strncpy(g_lang_code, "it", sizeof(g_lang_code) - 1); break;
        case I18N_LANG_JA: strncpy(g_lang_code, "ja", sizeof(g_lang_code) - 1); break;
        case I18N_LANG_ZH: strncpy(g_lang_code, "zh_CN", sizeof(g_lang_code) - 1); break;
        default:           strncpy(g_lang_code, "en", sizeof(g_lang_code) - 1); break;
    }
    load_locale(g_lang, g_lang_code);
}

i18n_lang_t i18n_get_lang(void) {
    return g_lang;
}

const char *i18n_get_lang_code(void) {
    return g_lang_code;
}

const char *i18n_tr(const char *msg) {
    if (!msg || !*msg) return "";

    /* 1. Try finding msg as a key in the dictionary */
    for (size_t i = 0; i < g_entry_count; i++) {
        if (strcmp(g_entries[i].key, msg) == 0) {
            return g_entries[i].val;
        }
    }

    /* 2. Try finding msg matching an English fallback string */
    size_t embed_count = sizeof(g_embedded_defaults) / sizeof(g_embedded_defaults[0]);
    for (size_t i = 0; i < embed_count; i++) {
        if (strcmp(g_embedded_defaults[i].en, msg) == 0) {
            const char *key = g_embedded_defaults[i].key;
            for (size_t j = 0; j < g_entry_count; j++) {
                if (strcmp(g_entries[j].key, key) == 0) {
                    return g_entries[j].val;
                }
            }
            return (g_lang == I18N_LANG_PT) ? g_embedded_defaults[i].pt : g_embedded_defaults[i].en;
        }
    }

    /* Fallback to original string */
    return msg;
}
