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
#include "../transaction/types.h"
#include "../common/bip32.h"
#include "../common/base64.h"
#include "../common/format_bigint.h"
#include "../common/format_address.h"
#include "menu.h"
#include "helpers/display_transaction.h"
#include "hint_buffers_nbgl.h"

static char g_operation[G_OPERATION_LEN];
static char g_amount[G_AMOUNT_LEN];
static char g_address[G_ADDRESS_LEN];
static char g_payload[G_PAYLOAD_LEN];
static char g_address_title[G_ADDRESS_TITLE_LEN];

static char g_transaction_title[64];

static nbgl_layoutTagValue_t pairs[3 + MAX_HINTS];
static nbgl_layoutTagValueList_t pairList;

static nbgl_pageInfoLongPress_t infoLongPress;

static void confirm_transaction_rejection(void) {
    // display a status page and go back to main
    ui_action_validate_transaction(false);
    nbgl_useCaseStatus("Transaction rejected", false, ui_menu_main);
}

static void ask_rejection_confirmation(void) {
    // display a choice to confirm/cancel rejection
    nbgl_useCaseConfirm("Reject transaction?",
                        NULL,
                        "Yes, reject",
                        "Go back to transaction",
                        confirm_transaction_rejection);
}

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void on_review_choice(bool confirm) {
    if (confirm) {
        // display a status page and go back to main
        ui_action_validate_transaction(true);
        nbgl_useCaseStatus("TRANSACTION\nSIGNED", true, ui_menu_main);
    } else {
        ask_rejection_confirmation();
    }
}

static void start_regular_review(void) {
    int pairIndex = 0;

    pairs[pairIndex].item = "Transaction type";
    pairs[pairIndex].value = g_operation;
    pairIndex++;

    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = g_amount;
    pairIndex++;

    pairs[pairIndex].item = g_address_title;
    pairs[pairIndex].value = g_address;
    pairIndex++;

    if (G_context.tx_info.transaction.has_payload && G_context.tx_info.transaction.is_blind) {
        pairs[pairIndex].item = "Payload";
        pairs[pairIndex].value = g_payload;
        pairIndex++;
    }

    print_hints(&G_context.tx_info.transaction.hints, &pairs[pairIndex]);

    pairList.pairs = pairs;
    pairList.nbPairs = pairIndex + G_context.tx_info.transaction.hints.hints_count;
    pairList.smallCaseForValue = false;

    snprintf(g_transaction_title,
             sizeof(g_transaction_title),
             "Sign transaction\nto %s",
             G_context.tx_info.transaction.action);

    infoLongPress.icon = &C_ledger_stax_ton_64;
    infoLongPress.text = g_transaction_title;
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", on_review_choice);
}

static void show_blind_warning_if_needed(void) {
    if (G_context.tx_info.transaction.is_blind) {
        nbgl_useCaseReviewStart(
            &C_Important_Circle_64px,
            "Blind Signing",
            "This transaction cannot be\nsecurely interpreted by Ledger\nStax. It might put "
            "your assets\nat risk.",
            "Reject transaction",
            start_regular_review,
            ask_rejection_confirmation);
    } else {
        start_regular_review();
    }
}

// Public function to start the transaction review
// - Check if the app is in the right state for transaction review
// - Format the amount and address strings in g_amount and g_address buffers
// - Display the first screen of the transaction review
int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    if (!display_transaction(g_operation,
                             sizeof(g_operation),
                             g_amount,
                             sizeof(g_amount),
                             g_address,
                             sizeof(g_address),
                             g_payload,
                             sizeof(g_payload),
                             g_address_title,
                             sizeof(g_address_title))) {
        return -1;
    }

    snprintf(g_transaction_title,
             sizeof(g_transaction_title),
             "Review transaction\nto %s",
             G_context.tx_info.transaction.action);

    // Start review
    nbgl_useCaseReviewStart(&C_ledger_stax_ton_64,
                            g_transaction_title,
                            NULL,
                            "Reject transaction",
                            show_blind_warning_if_needed,
                            ask_rejection_confirmation);

    return 0;
}

static void ui_blind_signing_error_choice(bool confirm) {
    if (confirm) {
        ui_menu_main();
    } else {
        ui_menu_settings();
    }
}

void ui_blind_signing_error() {
    nbgl_useCaseChoice(&C_Warning_64px,
                       "This message cannot\nbe clear-signed",
                       "Enable blind-signing in\nthe settings to sign\nthis transaction.",
                       "Exit",
                       "Go to settings",
                       ui_blind_signing_error_choice);
}

#endif
