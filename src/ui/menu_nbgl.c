/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#ifdef HAVE_NBGL

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"

#include "../globals.h"
#include "menu.h"

void app_quit(void) {
    // exit app here
    os_sched_exit(-1);
}

// 'About' menu

enum {
    BLIND_SIGNING_TOKEN = FIRST_USER_TOKEN,
    EXPERT_MODE_TOKEN,
};

static void controls_callback(int token, uint8_t index, int page) {
    (void) index;
    (void) page;
    bool value;
    switch (token) {
        case BLIND_SIGNING_TOKEN:
            value = N_storage.blind_signing_enabled ? false : true;
            nvm_write((void *) &N_storage.blind_signing_enabled, (void *) &value, sizeof(bool));
            break;
        case EXPERT_MODE_TOKEN:
            value = N_storage.expert_mode ? false : true;
            nvm_write((void *) &N_storage.expert_mode, (void *) &value, sizeof(bool));
            break;
    }
}

enum { MY_SWITCH_1_ID = 0, MY_SWITCH_2_ID, SETTINGS_SWITCHES_NB };

static nbgl_contentSwitch_t switches[SETTINGS_SWITCHES_NB] = {0};

static void initialize_switches() {
    switches[MY_SWITCH_1_ID].initState = N_storage.blind_signing_enabled ? ON_STATE : OFF_STATE;
    switches[MY_SWITCH_1_ID].text = "Blind signing";
    switches[MY_SWITCH_1_ID].subText = "Enable transaction blind\nsigning";
    switches[MY_SWITCH_1_ID].token = BLIND_SIGNING_TOKEN;
    switches[MY_SWITCH_1_ID].tuneId = TUNE_TAP_CASUAL;

    switches[MY_SWITCH_2_ID].initState = N_storage.expert_mode ? ON_STATE : OFF_STATE;
    switches[MY_SWITCH_2_ID].text = "Expert mode";
    switches[MY_SWITCH_2_ID].subText = "Show more information\nwhen reviewing transactions";
    switches[MY_SWITCH_2_ID].token = EXPERT_MODE_TOKEN;
    switches[MY_SWITCH_2_ID].tuneId = TUNE_TAP_CASUAL;
}

static const nbgl_content_t contents[] = {
    {
        .type = SWITCHES_LIST,
        .content.switchesList.switches = switches,
        .content.switchesList.nbSwitches = SETTINGS_SWITCHES_NB,
        .contentActionCallback = controls_callback,
    },
};

static const nbgl_genericContents_t settingsContents = {
    .callbackCallNeeded = false,
    .contentsList = contents,
    .nbContents = 1,
};

static const char *const INFO_TYPES[] = {"Version", "Developer"};
static const char *const INFO_CONTENTS[] = {APPVERSION, "TonTech"};

static const nbgl_contentInfoList_t infoContents = {
    .infoTypes = INFO_TYPES,
    .infoContents = INFO_CONTENTS,
    .nbInfos = 2,
};

static void ui_menu_common(uint8_t initPage) {
    initialize_switches();

    nbgl_useCaseHomeAndSettings(APPNAME,
                                &C_ledger_stax_ton_64,
                                NULL,
                                initPage,
                                &settingsContents,
                                &infoContents,
                                NULL,
                                app_quit);
}

void ui_menu_main(void) {
    ui_menu_common(INIT_HOME_PAGE);
}

void ui_menu_settings() {
    ui_menu_common(0);
}

void ui_menu_about() {
    ui_menu_common(1);
}

#endif
