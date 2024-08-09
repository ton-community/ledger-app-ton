#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"

#include "display.h"
#include "../constants.h"
#include "../globals.h"
#include "io.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/encoding.h"
#include "bip32.h"
#include "../common/base64.h"
#include "../common/format_address.h"
#include "menu.h"
#include "helpers/display_proof.h"

static nbgl_layoutTagValue_t pair;
static nbgl_layoutTagValueList_t pairList;
static char g_address[G_ADDRESS_LEN];
static char g_domain[MAX_DOMAIN_LEN + 1];

static void confirm_address_rejection(void) {
    // display a status page and go back to main
    ui_action_validate_proof(false);
    nbgl_useCaseStatus("Address verification\ncancelled", false, ui_menu_main);
}

static void confirm_address_approval(void) {
    // display a success status page and go back to main
    ui_action_validate_proof(true);
    nbgl_useCaseStatus("ADDRESS\nVERIFIED", true, ui_menu_main);
}

static void review_choice(bool confirm) {
    if (confirm) {
        confirm_address_approval();
    } else {
        confirm_address_rejection();
    }
}

static void continue_review(void) {
    pair.item = "App domain";
    pair.value = g_domain;

    pairList.nbPairs = 1;
    pairList.nbMaxLinesForValue = 0;
    pairList.pairs = &pair;

    nbgl_useCaseAddressConfirmationExt(g_address, review_choice, &pairList);
}

int ui_display_proof(uint8_t flags) {
    if (G_context.req_type != GET_PROOF || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    if (!display_proof(flags, g_address, sizeof(g_address), g_domain, sizeof(g_domain), NULL, 0)) {
        return -1;
    }

    nbgl_useCaseReviewStart(&C_ledger_stax_ton_64,
                            "Verify TON address\nto application",
                            NULL,
                            "Cancel",
                            continue_review,
                            confirm_address_rejection);

    return 0;
}

#endif
