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

void ui_menu_main(void) {
    nbgl_useCaseHome(APPNAME, &C_ledger_stax_ton_64, NULL, true, ui_menu_about, app_quit);
}

// 'About' menu

static const char *const INFO_TYPES[] = {"Version", "Developer"};
static const char *const INFO_CONTENTS[] = {APPVERSION, "TonTech"};

enum {
    BLIND_SIGNING_TOKEN = FIRST_USER_TOKEN,
    EXPERT_MODE_TOKEN,
};

static nbgl_layoutSwitch_t switches[2];

static bool nav_callback(uint8_t page, nbgl_pageContent_t *content) {
    switch (page) {
        case 0: {
            content->type = INFOS_LIST;
            content->infosList.nbInfos = 2;
            content->infosList.infoTypes = (const char **) INFO_TYPES;
            content->infosList.infoContents = (const char **) INFO_CONTENTS;
            break;
        }
        case 1: {
            int sw = 0;
            switches[sw++] = (nbgl_layoutSwitch_t){
                .initState = N_storage.blind_signing_enabled ? ON_STATE : OFF_STATE,
                .text = "Blind signing",
                .subText = "Enable transaction blind\nsigning",
                .token = BLIND_SIGNING_TOKEN,
                .tuneId = TUNE_TAP_CASUAL};
            switches[sw++] = (nbgl_layoutSwitch_t){
                .initState = N_storage.expert_mode ? ON_STATE : OFF_STATE,
                .text = "Expert mode",
                .subText = "Show more information\nwhen reviewing transactions",
                .token = EXPERT_MODE_TOKEN,
                .tuneId = TUNE_TAP_CASUAL};
            content->type = SWITCHES_LIST;
            content->switchesList.nbSwitches = sw;
            content->switchesList.switches = switches;
            break;
        }
        default: {
            return false;
        }
    }

    return true;
}

static void controls_callback(int token, uint8_t index) {
    (void) index;
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

static void ui_menu_settings_common() {
    nbgl_useCaseSettings(APPNAME " settings",
                         0,
                         2,
                         false,
                         ui_menu_main,
                         nav_callback,
                         controls_callback);
}

void ui_menu_settings() {
    ui_menu_settings_common();
}

void ui_menu_about() {
    ui_menu_settings_common();
}

#endif
