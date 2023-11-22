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
#include <string.h>   // memset, explicit_bzero
#include <stdbool.h>  // bool
#include <stddef.h>

#include "os.h"
#include "cx.h"

#include "crypto.h"

#include "constants.h"
#include "globals.h"
#include "common/write.h"

static int crypto_sign(const uint32_t *bip32_path,
                       uint8_t bip32_path_len,
                       const uint8_t *data,
                       size_t data_len,
                       uint8_t *sig,
                       size_t sig_len) {
    cx_ecfp_private_key_t private_key = {0};
    uint8_t raw_private_key[PRIVKEY_LEN] = {0};

    if (os_derive_bip32_with_seed_no_throw(HDW_ED25519_SLIP10,
                                           CX_CURVE_Ed25519,
                                           bip32_path,
                                           bip32_path_len,
                                           raw_private_key,
                                           NULL,
                                           (unsigned char *) "ed25519 seed",
                                           12) != CX_OK) {
        explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        return -1;
    }

    if (cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519, raw_private_key, 32, &private_key) !=
        CX_OK) {
        explicit_bzero(&private_key, sizeof(private_key));
        explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        return -1;
    }

    explicit_bzero(&raw_private_key, sizeof(raw_private_key));

    if (cx_eddsa_sign_no_throw(&private_key, CX_SHA512, data, data_len, sig, sig_len) != CX_OK) {
        explicit_bzero(&private_key, sizeof(private_key));
        return -1;
    }

    explicit_bzero(&private_key, sizeof(private_key));

    return 0;
}

int crypto_derive_public_key(const uint32_t *bip32_path,
                             uint8_t bip32_path_len,
                             uint8_t raw_public_key[static PUBKEY_LEN]) {
    cx_ecfp_private_key_t private_key = {0};
    uint8_t raw_private_key[PRIVKEY_LEN] = {0};
    cx_ecfp_public_key_t public_key = {0};

    if (os_derive_bip32_with_seed_no_throw(HDW_ED25519_SLIP10,
                                           CX_CURVE_Ed25519,
                                           bip32_path,
                                           bip32_path_len,
                                           raw_private_key,
                                           NULL,
                                           (unsigned char *) "ed25519 seed",
                                           12) != CX_OK) {
        explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        return -1;
    }

    if (cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519, raw_private_key, 32, &private_key) !=
        CX_OK) {
        explicit_bzero(&private_key, sizeof(private_key));
        explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        return -1;
    }

    explicit_bzero(&raw_private_key, sizeof(raw_private_key));

    if (cx_ecfp_generate_pair_no_throw(CX_CURVE_Ed25519, &public_key, &private_key, 1) != CX_OK) {
        explicit_bzero(&private_key, sizeof(private_key));
        return -1;
    }

    explicit_bzero(&private_key, sizeof(private_key));

    // Convert to NaCL format
    for (int i = 0; i < PUBKEY_LEN; i++) {
        raw_public_key[i] = public_key.W[64 - i];
    }
    if ((public_key.W[32] & 1) != 0) {
        raw_public_key[31] |= 0x80;
    }

    explicit_bzero(&public_key, sizeof(public_key));

    return 0;
}

int crypto_sign_tx() {
    if (crypto_sign(G_context.bip32_path,
                    G_context.bip32_path_len,
                    G_context.tx_info.m_hash,
                    sizeof(G_context.tx_info.m_hash),
                    G_context.tx_info.signature,
                    sizeof(G_context.tx_info.signature)) < 0) {
        return -1;
    }

    return 0;
}

int crypto_sign_proof() {
    if (crypto_sign(G_context.bip32_path,
                    G_context.bip32_path_len,
                    G_context.proof_info.hash,
                    sizeof(G_context.proof_info.hash),
                    G_context.proof_info.signature,
                    sizeof(G_context.proof_info.signature)) < 0) {
        return -1;
    }

    return 0;
}

int crypto_sign_sign_data() {
    uint8_t data[4 + 8 + HASH_LEN] = {0};
    write_u32_be(data, 0, G_context.sign_data_info.schema_crc);
    write_u64_be(data, 4, G_context.sign_data_info.timestamp);
    memmove(&data[4 + 8], G_context.sign_data_info.cell_hash, HASH_LEN);

    if (crypto_sign(G_context.bip32_path,
                    G_context.bip32_path_len,
                    data,
                    sizeof(data),
                    G_context.sign_data_info.signature,
                    sizeof(G_context.sign_data_info.signature)) < 0) {
        return -1;
    }

    return 0;
}
