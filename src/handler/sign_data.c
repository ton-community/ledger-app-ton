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

#include "sign_data.h"
#include "../sw.h"
#include "../globals.h"
#include "../crypto.h"
#include "../ui/display.h"
#include "common/mybuffer.h"
#include "../common/bip32_check.h"
#include "../sign_data/sign_data_deserialize.h"

int handler_sign_data(buffer_t *cdata, bool first, bool more) {
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

        G_context.req_type = CONFIRM_SIGN_DATA;
        G_context.state = STATE_NONE;

        return io_send_sw(SW_OK);
    }

    if (G_context.req_type != CONFIRM_SIGN_DATA) {
        return io_send_sw(SW_BAD_STATE);
    }

    if (G_context.sign_data_info.raw_data_len + cdata->size > MAX_DATA_LEN) {
        return io_send_sw(SW_WRONG_SIGN_DATA_LENGTH);
    }

    if (!buffer_move(cdata,
                     &G_context.sign_data_info.raw_data[G_context.sign_data_info.raw_data_len],
                     cdata->size)) {
        return io_send_sw(SW_WRONG_SIGN_DATA_LENGTH);
    }

    G_context.sign_data_info.raw_data_len += cdata->size;

    if (more) {
        return io_send_sw(SW_OK);
    }

    buffer_t buf = {.ptr = G_context.sign_data_info.raw_data,
                    .size = G_context.sign_data_info.raw_data_len,
                    .offset = 0};

    // Parse
    bool ok = sign_data_deserialize(&buf, &G_context.sign_data_info);
    if (!ok) {
        return io_send_sw(SW_SIGN_DATA_PARSING_FAIL);
    }

    G_context.state = STATE_PARSED;

    return ui_display_sign_data();
}
