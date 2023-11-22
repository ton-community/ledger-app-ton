#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"

#include "display.h"
#include "../constants.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/bip32.h"
#include "../common/base64.h"
#include "menu.h"
#include "helpers/display_address.h"

static char g_address[G_ADDRESS_LEN];

static void confirm_address_rejection(void) {
    // display a status page and go back to main
    ui_action_validate_pubkey(false);
    nbgl_useCaseStatus("Address verification\ncancelled", false, ui_menu_main);
}

static void confirm_address_approval(void) {
    // display a success status page and go back to main
    ui_action_validate_pubkey(true);
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
    nbgl_useCaseAddressConfirmation(g_address, review_choice);
}

int ui_display_address(uint8_t flags) {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format address
    if (!display_address(flags, g_address, sizeof(g_address), NULL, 0)) {
        return -1;
    }

    nbgl_useCaseReviewStart(&C_ledger_stax_ton_64,
                            "Verify TON address",
                            NULL,
                            "Cancel",
                            continue_review,
                            confirm_address_rejection);

    return 0;
}

#endif
