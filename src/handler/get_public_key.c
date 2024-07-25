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

#include "get_public_key.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../crypto.h"
#include "../common/buffer.h"
#include "../common/bip32_check.h"
#include "../ui/display.h"
#include "../helper/send_response.h"
#include "../apdu/params.h"

int handler_get_public_key(uint8_t flags, buffer_t *cdata, bool display) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;

    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    if (!check_global_bip32_path()) {
        return io_send_sw(SW_BAD_BIP32_PATH);
    }

    if (crypto_derive_public_key(G_context.bip32_path,
                                 G_context.bip32_path_len,
                                 G_context.pk_info.raw_public_key) < 0) {
        return io_send_sw(SW_BAD_STATE);
    }

    if (flags & P2_ADDR_FLAG_WALLET_SPECIFIERS) {
        if (!buffer_read_bool(cdata, &G_context.pk_info.is_v3r2) ||
            !buffer_read_u32(cdata, &G_context.pk_info.subwallet_id, BE)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }
    } else {
        G_context.pk_info.subwallet_id = 698983191;
        G_context.pk_info.is_v3r2 = false;
    }

    if (display) {
        return ui_display_address(flags);
    }

    return helper_send_response_pubkey();
}
