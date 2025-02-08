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

#include "deserialize.h"
#include "types.h"
#include "common/mybuffer.h"
#include "hash.h"
#include "../common/cell.h"
#include "transaction_hints.h"
#include "../constants.h"
#include "../common/types.h"

#define SAFE(RES, CODE) \
    if (!RES) {         \
        return CODE;    \
    }

typedef struct {
    uint32_t id;
    char name[8];
    uint8_t decimals;
} extra_currency_t;

static const extra_currency_t extra_currencies[] = {
    {
        .id = 0,
        .name = "tgBTC",
        .decimals = 9,
    },
};

parser_status_e transaction_deserialize(buffer_t *buf, transaction_t *tx) {
    if (buf->size > MAX_TRANSACTION_LEN) {
        return WRONG_LENGTH_ERROR;
    }

    // tag
    SAFE(buffer_read_u8(buf, &tx->tag), TAG_PARSING_ERROR);
    if (tx->tag >= 0x02) {  // Only 0x00 and 0x01 are supported now
        return TAG_PARSING_ERROR;
    }

    tx->hints.hints_count = 0;

    if (tx->tag == 0x01) {
        SAFE(buffer_read_u32(buf, &tx->subwallet_id, BE), GENERAL_ERROR);
        SAFE(buffer_read_u8(buf, &tx->flags), GENERAL_ERROR);
    } else {
        tx->subwallet_id = DEFAULT_SUBWALLET_ID;
        tx->flags = TX_INCLUDE_WALLET_OP_BIT;
    }

    // Basic Transaction parameters
    SAFE(buffer_read_u32(buf, &tx->seqno, BE), SEQ_PARSING_ERROR);
    SAFE(buffer_read_u32(buf, &tx->timeout, BE), TIMEOUT_PARSING_ERROR);
    SAFE(buffer_read_varuint(buf, &tx->value_len, tx->value_buf, MAX_VALUE_BYTES_LEN),
         VALUE_PARSING_ERROR);
    if (transaction_include_extra_currency(tx)) {
        uint8_t ec_idx;
        SAFE(buffer_read_u8(buf, &ec_idx), EXTRA_CURRENCY_PARSING_ERROR);
        if (ec_idx >= sizeof(extra_currencies) / sizeof(extra_currencies[0])) {
            return EXTRA_CURRENCY_PARSING_ERROR;
        }

        tx->extra_currency_id = extra_currencies[ec_idx].id;
        SAFE(buffer_read_varuint(buf, &tx->extra_currency_amount_len, tx->extra_currency_amount_buf, MAX_EXTRA_CURRENCY_AMOUNT_BYTES_LEN),
             EXTRA_CURRENCY_PARSING_ERROR);

        add_hint_amount(&tx->hints, "Extra currency", extra_currencies[ec_idx].name, tx->extra_currency_amount_buf, tx->extra_currency_amount_len, extra_currencies[ec_idx].decimals);
    }
    SAFE(buffer_read_address(buf, &tx->to), TO_PARSING_ERROR);
    SAFE(buffer_read_bool(buf, &tx->bounce), BOUNCE_PARSING_ERROR);
    SAFE(buffer_read_u8(buf, &tx->send_mode), SEND_MODE_PARSING_ERROR);

    // state-init
    SAFE(buffer_read_bool(buf, &tx->has_state_init), STATE_INIT_PARSING_ERROR);
    if (tx->has_state_init) {
        SAFE(buffer_read_cell_ref(buf, &tx->state_init), STATE_INIT_PARSING_ERROR);
    }

    // Payload
    SAFE(buffer_read_bool(buf, &tx->has_payload), PAYLOAD_PARSING_ERROR);
    if (tx->has_payload) {
        SAFE(buffer_read_cell_ref(buf, &tx->payload), STATE_INIT_PARSING_ERROR);
    }

    // Hints
    SAFE(buffer_read_bool(buf, &tx->has_hints), HINTS_PARSING_ERROR);
    if (tx->has_hints) {
        if (!tx->has_payload) {
            return HINTS_PARSING_ERROR;
        }
        SAFE(buffer_read_u32(buf, &tx->hints_type, BE), HINTS_PARSING_ERROR);
        SAFE(buffer_read_u16(buf, &tx->hints_len, BE), HINTS_PARSING_ERROR);
        SAFE(buffer_read_ref(buf, &tx->hints_data, tx->hints_len), HINTS_PARSING_ERROR);
    }

    // Process hints
    SAFE(process_hints(tx), HINTS_PARSING_ERROR);

    if (tx->subwallet_id != DEFAULT_SUBWALLET_ID) {
        add_hint_number(&tx->hints, "Subwallet ID", (uint64_t) tx->subwallet_id);
    }

    return (buf->offset == buf->size) ? PARSING_OK : WRONG_LENGTH_ERROR;
}
