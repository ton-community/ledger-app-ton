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
#include "bip32.h"
#include "../common/base64.h"
#include "menu.h"
#include "helpers/display_address.h"

static char g_address[G_ADDRESS_LEN];

static void review_choice(bool confirm) {
    ui_action_validate_pubkey(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
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

    nbgl_useCaseAddressReview(g_address, NULL, &C_ledger_stax_ton_64, "Verify TON address", NULL, review_choice);

    return 0;
}

#endif
