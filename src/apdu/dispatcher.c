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

#include <stdint.h>
#include <stdbool.h>

#include "dispatcher.h"
#include "params.h"
#include "../constants.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../common/buffer.h"
#include "../handler/get_address_proof.h"
#include "../handler/get_version.h"
#include "../handler/get_app_name.h"
#include "../handler/get_public_key.h"
#include "../handler/sign_tx.h"
#include "../handler/sign_data.h"

int apdu_dispatcher(const command_t *cmd) {
    if (cmd->cla != CLA) {
        return io_send_sw(SW_CLA_NOT_SUPPORTED);
    }

    buffer_t buf = {0};

    switch (cmd->ins) {
        case GET_VERSION:
            if (cmd->p1 != P1_NONE || cmd->p2 != P2_NONE) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            return handler_get_version();
        case GET_APP_NAME:
            if (cmd->p1 != P1_NONE || cmd->p2 != P2_NONE) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            return handler_get_app_name();
        case GET_PUBLIC_KEY:
            if (!(cmd->p1 == P1_NON_CONFIRM || cmd->p1 == P1_CONFIRM) ||
                cmd->p2 > P2_ADDR_FLAGS_MAX) {
                return io_send_sw(SW_WRONG_P1P2);
            }
            if (cmd->p1 == P1_NON_CONFIRM && cmd->p2 != P2_NONE) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_get_public_key(cmd->p2, &buf, (bool) cmd->p1);
        case SIGN_TX:
            if (cmd->p1 != P1_NONE) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (cmd->p2 & ~(P2_FIRST | P2_MORE)) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if ((cmd->p2 & P2_FIRST) && !(cmd->p2 & P2_MORE)) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_sign_tx(&buf, (bool) (cmd->p2 & P2_FIRST), (bool) (cmd->p2 & P2_MORE));
        case GET_ADDRESS_PROOF:
            if (cmd->p1 != P1_CONFIRM) {
                return io_send_sw(SW_WRONG_P1P2);
            }
            if (cmd->p2 > P2_ADDR_FLAGS_MAX) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_get_address_proof(cmd->p2, &buf);
        case SIGN_DATA:
            if (cmd->p1 != P1_NONE) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (cmd->p2 & ~(P2_FIRST | P2_MORE)) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if ((cmd->p2 & P2_FIRST) && !(cmd->p2 & P2_MORE)) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_sign_data(&buf, (bool) (cmd->p2 & P2_FIRST), (bool) (cmd->p2 & P2_MORE));
        default:
            return io_send_sw(SW_INS_NOT_SUPPORTED);
    }
}
