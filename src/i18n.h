#ifndef I18N_H
#define I18N_H

typedef enum {
    I18N_LANG_AUTO = 0,
    I18N_LANG_EN,
    I18N_LANG_PT,
    I18N_LANG_ES,
    I18N_LANG_FR,
    I18N_LANG_DE,
    I18N_LANG_IT,
    I18N_LANG_JA,
    I18N_LANG_ZH
} i18n_lang_t;

/* Initialize i18n. If lang_str is NULL or "auto", detects from system environment. */
void i18n_init(const char *lang_str);

/* Explicitly set language */
void i18n_set_lang(i18n_lang_t lang);

/* Get current active language enum */
i18n_lang_t i18n_get_lang(void);

/* Get current active language code string (e.g. "en", "pt", "es", "fr", "de", "it", "ja", "zh") */
const char *i18n_get_lang_code(void);

/* Translate string (by key or English fallback) */
const char *i18n_tr(const char *msg);

/* Convenient shortcut macro */
#define _(msg) i18n_tr(msg)

#endif /* I18N_H */
