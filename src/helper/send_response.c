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

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <string.h>  // memmove

#include "send_response.h"
#include "../constants.h"
#include "../globals.h"
#include "../sw.h"
#include "common/mybuffer.h"

int helper_send_response_pubkey() {
    uint8_t resp[PUBKEY_LEN] = {0};
    size_t offset = 0;

    memmove(resp + offset, G_context.pk_info.raw_public_key, PUBKEY_LEN);
    offset += PUBKEY_LEN;

    return io_send_response_buffer(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0},
                                   SW_OK);
}

int helper_send_response_sig() {
    uint8_t resp[1 + SIG_LEN + 1 + HASH_LEN] = {0};
    size_t offset = 0;

    // Signature
    resp[offset++] = SIG_LEN;
    memmove(resp + offset, G_context.tx_info.signature, SIG_LEN);
    offset += SIG_LEN;

    // Hash
    resp[offset++] = HASH_LEN;
    memmove(resp + offset, G_context.tx_info.m_hash, HASH_LEN);
    offset += HASH_LEN;

    return io_send_response_buffer(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0},
                                   SW_OK);
}

int helper_send_response_sig_proof() {
    uint8_t resp[1 + SIG_LEN + 1 + HASH_LEN] = {0};
    size_t offset = 0;

    // Signature
    resp[offset++] = SIG_LEN;
    memmove(resp + offset, G_context.proof_info.signature, SIG_LEN);
    offset += SIG_LEN;

    // Hash
    resp[offset++] = HASH_LEN;
    memmove(resp + offset, G_context.proof_info.hash, HASH_LEN);
    offset += HASH_LEN;

    return io_send_response_buffer(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0},
                                   SW_OK);
}

int helper_send_response_sig_sign_data() {
    uint8_t resp[1 + SIG_LEN + 1 + HASH_LEN] = {0};
    size_t offset = 0;

    // Signature
    resp[offset++] = SIG_LEN;
    memmove(resp + offset, G_context.sign_data_info.signature, SIG_LEN);
    offset += SIG_LEN;

    // Hash
    resp[offset++] = HASH_LEN;
    memmove(resp + offset, G_context.sign_data_info.cell_hash, HASH_LEN);
    offset += HASH_LEN;

    return io_send_response_buffer(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0},
                                   SW_OK);
}
