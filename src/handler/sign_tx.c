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

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"

#include "sign_tx.h"
#include "../sw.h"
#include "../globals.h"
#include "../crypto.h"
#include "../ui/display.h"
#include "../common/buffer.h"
#include "../common/bip32_check.h"
#include "../transaction/types.h"
#include "../transaction/deserialize.h"
#include "../transaction/hash.h"

int handler_sign_tx(buffer_t *cdata, bool first, bool more) {
    if (first) {  // first APDU, parse BIP32 path
        explicit_bzero(&G_context, sizeof(G_context));

        if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
            !buffer_read_bip32_path(cdata,
                                    G_context.bip32_path,
                                    (size_t) G_context.bip32_path_len)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }

        if (!check_global_bip32_path()) {
            return io_send_sw(SW_BAD_BIP32_PATH);
        }

        G_context.req_type = CONFIRM_TRANSACTION;
        G_context.state = STATE_NONE;

        return io_send_sw(SW_OK);
    }

    if (G_context.req_type != CONFIRM_TRANSACTION) {
        return io_send_sw(SW_BAD_STATE);
    }

    if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN) {
        return io_send_sw(SW_WRONG_TX_LENGTH);
    }

    if (!buffer_move(cdata, &G_context.tx_info.raw_tx[G_context.tx_info.raw_tx_len], cdata->size)) {
        return io_send_sw(SW_WRONG_TX_LENGTH);
    }

    G_context.tx_info.raw_tx_len += cdata->size;

    if (more) {
        return io_send_sw(SW_OK);
    }

    buffer_t buf = {.ptr = G_context.tx_info.raw_tx,
                    .size = G_context.tx_info.raw_tx_len,
                    .offset = 0};

    // Parse
    parser_status_e status = transaction_deserialize(&buf, &G_context.tx_info.transaction);
    PRINTF("Parsing status: %d.\n", status);
    if (status != PARSING_OK) {
        return io_send_sw(SW_TX_PARSING_FAIL);
    }

    if (G_context.tx_info.transaction.is_blind && !N_storage.blind_signing_enabled) {
        ui_blind_signing_error();
        return io_send_sw(SW_BLIND_SIGNING_DISABLED);
    }

    // Hash
    if (!hash_tx(&G_context.tx_info)) {
        return io_send_sw(SW_TX_PARSING_FAIL);
    }

    G_context.state = STATE_PARSED;

    PRINTF("Hash: %.*H\n", sizeof(G_context.tx_info.m_hash), G_context.tx_info.m_hash);

    return ui_display_transaction();
}
