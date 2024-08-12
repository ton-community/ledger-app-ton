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

static nbgl_contentTagValue_t pair;
static nbgl_contentTagValueList_t pairList;
static char g_address[G_ADDRESS_LEN];
static char g_domain[MAX_DOMAIN_LEN + 1];

static void review_choice(bool confirm) {
    ui_action_validate_proof(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
}

int ui_display_proof(uint8_t flags) {
    if (G_context.req_type != GET_PROOF || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    if (!display_proof(flags, g_address, sizeof(g_address), g_domain, sizeof(g_domain), NULL, 0)) {
        return -1;
    }

    pair.item = "App domain";
    pair.value = g_domain;

    pairList.nbPairs = 1;
    pairList.nbMaxLinesForValue = 0;
    pairList.pairs = &pair;

    nbgl_useCaseAddressReview(g_address, &pairList, &C_ledger_stax_ton_64, "Verify TON address\nto application", NULL, review_choice);

    return 0;
}

#endif
