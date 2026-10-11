#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/cli_parse.h"
#include "../src/scarlett_alsa.h"
#include "../src/i18n.h"

int main(void) {
    printf("[TEST] Executando testes unitários do parser CLI...\n");

    /* Testes parse_bool_val */
    assert(parse_bool_val("1") == 1);
    assert(parse_bool_val("on") == 1);
    assert(parse_bool_val("ON") == 1);
    assert(parse_bool_val("true") == 1);
    assert(parse_bool_val("TRUE") == 1);
    assert(parse_bool_val("yes") == 1);
    assert(parse_bool_val("YES") == 1);

    assert(parse_bool_val("0") == 0);
    assert(parse_bool_val("off") == 0);
    assert(parse_bool_val("OFF") == 0);
    assert(parse_bool_val("false") == 0);
    assert(parse_bool_val("FALSE") == 0);
    assert(parse_bool_val("no") == 0);
    assert(parse_bool_val("NO") == 0);

    assert(parse_bool_val(NULL) == -1);
    assert(parse_bool_val("") == -1);
    assert(parse_bool_val("invalid") == -1);
    assert(parse_bool_val("2") == -1);
    assert(parse_bool_val("maybe") == -1);

    /* Testes parse_monitor_mode */
    assert(parse_monitor_mode("off") == SCARLETT_MONITOR_OFF);
    assert(parse_monitor_mode("OFF") == SCARLETT_MONITOR_OFF);
    assert(parse_monitor_mode("0") == SCARLETT_MONITOR_OFF);

    assert(parse_monitor_mode("mono") == SCARLETT_MONITOR_MONO);
    assert(parse_monitor_mode("MONO") == SCARLETT_MONITOR_MONO);
    assert(parse_monitor_mode("1") == SCARLETT_MONITOR_MONO);

    assert(parse_monitor_mode("stereo") == SCARLETT_MONITOR_STEREO);
    assert(parse_monitor_mode("STEREO") == SCARLETT_MONITOR_STEREO);
    assert(parse_monitor_mode("2") == SCARLETT_MONITOR_STEREO);

    assert(parse_monitor_mode(NULL) == -1);
    assert(parse_monitor_mode("") == -1);
    assert(parse_monitor_mode("quad") == -1);
    assert(parse_monitor_mode("3") == -1);
    assert(parse_monitor_mode("surround") == -1);

    /* Testes i18n */
    printf("[TEST] Executando testes unitários de internacionalização (i18n)...\n");

    /* Inglês */
    i18n_init("en");
    assert(i18n_get_lang() == I18N_LANG_EN);
    assert(strcmp(_("Quit"), "Quit") == 0);
    assert(strcmp(_("Level:"), "Level:") == 0);

    /* Português */
    i18n_init("pt");
    assert(i18n_get_lang() == I18N_LANG_PT);
    assert(strcmp(_("Quit"), "Sair") == 0);
    assert(strcmp(_("Level:"), "Nível:") == 0);
    assert(strcmp(_("[ Input 1 ]"), "[ Entrada 1 ]") == 0);

    /* pt_BR */
    i18n_init("pt_BR.UTF-8");
    assert(i18n_get_lang() == I18N_LANG_PT);
    assert(strcmp(_("Quit"), "Sair") == 0);
    assert(strcmp(_("cli_input1_sec"), " [ Entrada 1 ]\n") == 0);
    assert(strstr(_("err_specify_toggle"), "especifique") != NULL);

    /* Espanhol (es) */
    i18n_init("es");
    assert(i18n_get_lang() == I18N_LANG_ES);
    assert(strcmp(_("Quit"), "Salir") == 0);
    assert(strcmp(_("usage_word_usage"), "Uso") == 0);

    /* Francês (fr) */
    i18n_init("fr");
    assert(i18n_get_lang() == I18N_LANG_FR);
    assert(strcmp(_("Quit"), "Quitter") == 0);
    assert(strcmp(_("usage_word_usage"), "Utilisation") == 0);

    /* Alemão (de) */
    i18n_init("de");
    assert(i18n_get_lang() == I18N_LANG_DE);
    assert(strcmp(_("Quit"), "Beenden") == 0);
    assert(strcmp(_("usage_word_usage"), "Verwendung") == 0);

    /* Italiano (it) */
    i18n_init("it");
    assert(i18n_get_lang() == I18N_LANG_IT);
    assert(strcmp(_("Quit"), "Esci") == 0);

    /* Japonês (ja) */
    i18n_init("ja");
    assert(i18n_get_lang() == I18N_LANG_JA);
    assert(strcmp(_("Quit"), "終了") == 0);

    /* Chinês Simplificado (zh) */
    i18n_init("zh_CN");
    assert(i18n_get_lang() == I18N_LANG_ZH);
    assert(strcmp(_("Quit"), "退出") == 0);

    /* Fallback para texto não cadastrado */
    assert(strcmp(_("Unmapped String 123"), "Unmapped String 123") == 0);

    printf("[TEST] Todos os testes unitários passaram com sucesso!\n");
    return 0;
}

