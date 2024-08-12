#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "os_io_seproxyhal.h"
#include "nbgl_use_case.h"

#include "display.h"
#include "../constants.h"
#include "../globals.h"
#include "io.h"
#include "../sw.h"
#include "action/validate.h"
#include "menu.h"
#include "hint_buffers_nbgl.h"

static nbgl_contentTagValue_t pairs[MAX_HINTS];
static nbgl_contentTagValueList_t pairList;

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void on_review_choice(bool confirm) {
    ui_action_validate_sign_data(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_MESSAGE_SIGNED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_MESSAGE_REJECTED, ui_menu_main);
    }
}

int ui_display_sign_data() {
    if (G_context.req_type != CONFIRM_SIGN_DATA || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    print_hints(&G_context.sign_data_info.hints, pairs);

    pairList.pairs = pairs;
    pairList.nbPairs = G_context.sign_data_info.hints.hints_count;
    pairList.smallCaseForValue = false;

    nbgl_useCaseReview(TYPE_MESSAGE,
                       &pairList,
                       &C_ledger_stax_ton_64,
                       "Sign custom data",
                       NULL,
                       "Sign custom data",
                       on_review_choice);

    return 0;
}

#endif
