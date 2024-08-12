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
#include "../transaction/types.h"
#include "bip32.h"
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
static char g_transaction_finish_title[64];

static nbgl_contentTagValue_t pairs[3 + MAX_HINTS];
static nbgl_contentTagValueList_t pairList;

static void on_transaction_accepted() {
    ui_action_validate_transaction(true);
    nbgl_useCaseReviewStatus(STATUS_TYPE_TRANSACTION_SIGNED, ui_menu_main);
}

static void on_transaction_rejected() {
    ui_action_validate_transaction(false);
    nbgl_useCaseReviewStatus(STATUS_TYPE_TRANSACTION_REJECTED, ui_menu_main);
}

static void on_review_choice(bool confirm) {
    if (confirm) {
        on_transaction_accepted();
    } else {
        on_transaction_rejected();
    }
}

static void ui_start_review() {
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
        io_send_sw(SW_BAD_STATE);
        return;
    }

    snprintf(g_transaction_title,
             sizeof(g_transaction_title),
             "Review transaction\nto %s",
             G_context.tx_info.transaction.action);

    snprintf(g_transaction_finish_title,
             sizeof(g_transaction_finish_title),
             "Sign transaction\nto %s",
             G_context.tx_info.transaction.action);

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

    nbgl_opType_t op = TYPE_TRANSACTION;
    if (G_context.tx_info.transaction.is_blind) {
        op |= BLIND_OPERATION;
    }

    nbgl_useCaseReview(op,
                       &pairList,
                       &C_ledger_stax_ton_64,
                       g_transaction_title,
                       NULL,
                       g_transaction_finish_title,
                       on_review_choice);
}

static void on_blind_choice2(bool proceed) {
    if (proceed) {
        ui_start_review();
    } else {
        on_transaction_rejected();
    }
}

static void on_blind_choice1(bool back_to_safety) {
    if (back_to_safety) {
        on_transaction_rejected();
    } else {
        nbgl_useCaseChoice(
            NULL,
            "Blind Signing",
            "This transaction cannot be\nsecurely interpreted by Ledger. It might put "
            "your assets\nat risk.",
            "I accept the risk",
            "Reject transaction",
            on_blind_choice2);
    }
}

static void ui_show_blind_warning() {
    nbgl_useCaseChoice(
        &C_Warning_64px,
        "Security risk detected",
        "It may not be safe to sign this transaction. To continue, you'll need to review the risk.",
        "Back to safety",
        "Review risk",
        on_blind_choice1);
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

    if (G_context.tx_info.transaction.is_blind) {
        ui_show_blind_warning();
    } else {
        ui_start_review();
    }

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
