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

#ifdef HAVE_BAGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "display.h"
#include "menu.h"
#include "../constants.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/bip32.h"
#include "../common/base64.h"
#include "../common/format_bigint.h"
#include "../common/format_address.h"
#include "../common/encoding.h"
#include "../common/hints.h"
#include "helpers/display_address.h"
#include "helpers/display_proof.h"
#include "helpers/display_transaction.h"

static action_validate_cb g_validate_callback;
static char g_operation[G_OPERATION_LEN];
static char g_amount[G_AMOUNT_LEN];
static char g_address[G_ADDRESS_LEN];
static char g_payload[G_PAYLOAD_LEN];
static char g_domain[MAX_DOMAIN_LEN + 1];
static char g_address_title[G_ADDRESS_TITLE_LEN];

const ux_flow_step_t *ux_approval_flow[64];

static HintHolder_t *g_hint_holder;
static int g_hint_offset;
static char g_hint_title[HINT_TITLE_SIZE];
static char g_hint_body[HINT_BODY_SIZE];

// Step with icon and text
UX_STEP_NOCB(ux_display_confirm_addr_step, pn, {&C_icon_eye, "Confirm Address"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_address_step,
             bnnn_paging,
             {
                 .title = g_address_title,
                 .text = g_address,
             });
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// FLOW to display address and BIP32 path:
// #1 screen: eye icon + "Confirm Address"
// #2 screen: display address
// #3 screen: approve button
// #4 screen: reject button
UX_FLOW(ux_display_pubkey_flow,
        &ux_display_confirm_addr_step,
        &ux_display_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_address(uint8_t flags) {
    // Check state
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format address
    if (!display_address(flags,
                         g_address,
                         sizeof(g_address),
                         g_address_title,
                         sizeof(g_address_title))) {
        return -1;
    }

    // Launch
    g_validate_callback = &ui_action_validate_pubkey;
    ux_flow_init(0, ux_display_pubkey_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_verify_addr_step, pn, {&C_icon_eye, "Verify Address"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_domain_step,
             bnnn_paging,
             {
                 .title = "App domain",
                 .text = g_domain,
             });

// FLOW to display address and BIP32 path:
// #1 screen: eye icon + "Verify Address"
// #2 screen: display app domain
// #3 screen: display address
// #4 screen: approve button
// #5 screen: reject button
UX_FLOW(ux_display_proof_flow,
        &ux_display_verify_addr_step,
        &ux_display_domain_step,
        &ux_display_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_proof(uint8_t flags) {
    // Check state
    if (G_context.req_type != GET_PROOF || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    if (!display_proof(flags,
                       g_address,
                       sizeof(g_address),
                       g_domain,
                       sizeof(g_domain),
                       g_address_title,
                       sizeof(g_address_title))) {
        return -1;
    }

    // Launch
    g_validate_callback = &ui_action_validate_proof;
    ux_flow_init(0, ux_display_proof_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 g_operation,
             });
// Blind signing warning
UX_STEP_NOCB(ux_display_blind_signing_warning_step,
             pbb,
             {
                 &C_icon_warning,
                 "Blind",
                 "Signing",
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_amount_step,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_amount,
             });
// Step with payload amount
UX_STEP_NOCB(ux_display_payload_step,
             bnnn_paging,
             {
                 .title = "Payload",
                 .text = g_payload,
             });
// Hints
UX_STEP_NOCB_INIT(ux_display_hint_step,
                  bnnn_paging,
                  {
                      size_t step_index = G_ux.flow_stack[stack_slot].index;
                      print_hint(g_hint_holder,
                                 step_index + g_hint_offset,
                                 g_hint_title,
                                 sizeof(g_hint_title),
                                 g_hint_body,
                                 sizeof(g_hint_body));
                  },
                  {
                      .title = g_hint_title,
                      .text = g_hint_body,
                  });

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

    // Configure Flow
    int step = 0;
    ux_approval_flow[step++] = &ux_display_review_step;
    if (G_context.tx_info.transaction.is_blind) {
        ux_approval_flow[step++] = &ux_display_blind_signing_warning_step;
    }
    ux_approval_flow[step++] = &ux_display_address_step;
    ux_approval_flow[step++] = &ux_display_amount_step;
    if (G_context.tx_info.transaction.has_payload && G_context.tx_info.transaction.is_blind) {
        ux_approval_flow[step++] = &ux_display_payload_step;
    }
    g_hint_holder = &G_context.tx_info.transaction.hints;
    g_hint_offset = -step;
    for (uint16_t i = 0; i < G_context.tx_info.transaction.hints.hints_count; i++) {
        ux_approval_flow[step++] = &ux_display_hint_step;
    }
    ux_approval_flow[step++] = &ux_display_approve_step;
    ux_approval_flow[step++] = &ux_display_reject_step;
    ux_approval_flow[step++] = FLOW_END_STEP;

    // Start flow
    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_approval_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_sign_custom_data_step,
             pnn,
             {
                 &C_icon_eye,
                 "Sign",
                 "Custom data",
             });

int ui_display_sign_data() {
    if (G_context.req_type != CONFIRM_SIGN_DATA || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Configure Flow
    int step = 0;
    ux_approval_flow[step++] = &ux_display_sign_custom_data_step;
    g_hint_holder = &G_context.sign_data_info.hints;
    g_hint_offset = -1;
    for (uint16_t i = 0; i < G_context.sign_data_info.hints.hints_count; i++) {
        ux_approval_flow[step++] = &ux_display_hint_step;
    }
    ux_approval_flow[step++] = &ux_display_approve_step;
    ux_approval_flow[step++] = &ux_display_reject_step;
    ux_approval_flow[step++] = FLOW_END_STEP;

    // Start flow
    g_validate_callback = &ui_action_validate_sign_data;
    ux_flow_init(0, ux_approval_flow, NULL);

    return 0;
}

#ifdef TARGET_NANOS
UX_STEP_CB(ux_warning_contract_data_step,
           bnnn_paging,
           ui_menu_main(),
           {
               "Error",
               "Blind signing must be enabled in Settings",
           });
#else
UX_STEP_CB(ux_warning_contract_data_step,
           pnn,
           ui_menu_main(),
           {
               &C_icon_crossmark,
               "Blind signing must be",
               "enabled in Settings",
           });
#endif

UX_FLOW(ux_warning_contract_data_flow, &ux_warning_contract_data_step);

void ui_blind_signing_error() {
    ux_flow_init(0, ux_warning_contract_data_flow, NULL);
}

#endif
