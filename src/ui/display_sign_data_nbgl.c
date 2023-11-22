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
#include "../io.h"
#include "../sw.h"
#include "action/validate.h"
#include "menu.h"
#include "hint_buffers_nbgl.h"

static nbgl_layoutTagValue_t pairs[MAX_HINTS];
static nbgl_layoutTagValueList_t pairList;

static nbgl_pageInfoLongPress_t infoLongPress;

static void confirm_sign_data_rejection(void) {
    // display a status page and go back to main
    ui_action_validate_sign_data(false);
    nbgl_useCaseStatus("Data not signed", false, ui_menu_main);
}

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void on_review_choice(bool confirm) {
    if (confirm) {
        // display a status page and go back to main
        ui_action_validate_sign_data(true);
        nbgl_useCaseStatus("DATA\nSIGNED", true, ui_menu_main);
    } else {
        confirm_sign_data_rejection();
    }
}

static void start_regular_review(void) {
    print_hints(&G_context.sign_data_info.hints, pairs);

    pairList.pairs = pairs;
    pairList.nbPairs = G_context.sign_data_info.hints.hints_count;
    pairList.smallCaseForValue = false;

    infoLongPress.icon = &C_ledger_stax_ton_64;
    infoLongPress.text = "Sign custom data";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Cancel", on_review_choice);
}

int ui_display_sign_data() {
    if (G_context.req_type != CONFIRM_SIGN_DATA || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Start review
    nbgl_useCaseReviewStart(&C_ledger_stax_ton_64,
                            "Sign custom data",
                            NULL,
                            "Cancel",
                            start_regular_review,
                            confirm_sign_data_rejection);

    return 0;
}

#endif
