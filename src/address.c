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
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "cx.h"

#include "address.h"

#include "transaction/types.h"
#include "common/crc16.h"
#include "common/format_address.h"
#include "constants.h"

#define SAFE(RES)         \
    if ((RES) != CX_OK) { \
        return false;     \
    }

const uint8_t root_header[] = {
    // Cell data and refs descriptor
    0x02,
    0x01,
    0x34,
    0x00,
    0x07,
    0x00,
    0x00,
    // Code cell hash
    0xfe,
    0xb5,
    0xff,
    0x68,
    0x20,
    0xe2,
    0xff,
    0x0d,
    0x94,
    0x83,
    0xe7,
    0xe0,
    0xd6,
    0x2c,
    0x81,
    0x7d,
    0x84,
    0x67,
    0x89,
    0xfb,
    0x4a,
    0xe5,
    0x80,
    0xc8,
    0x78,
    0x86,
    0x6d,
    0x95,
    0x9d,
    0xab,
    0xd5,
    0xc0};

const uint8_t v3r2_root_header[] = { 0x2, 0x1, 0x34, 0x0, 0x0, 0x0, 0x0, 0x84, 0xda, 0xfa, 0x44, 0x9f, 0x98, 0xa6, 0x98, 0x77, 0x89, 0xba, 0x23, 0x23, 0x58, 0x7, 0x2b, 0xc0, 0xf7, 0x6d, 0xc4, 0x52, 0x40, 0x2, 0xa5, 0xd0, 0x91, 0x8b, 0x9a, 0x75, 0xd2, 0xd5, 0x99 };

const uint8_t data_header[] = {
    0x00,
    0x51,  // Cell header
    0x00,
    0x00,
    0x00,
    0x00,  // Seqno
};

const uint8_t v3r2_data_header[] = {
    0x00,
    0x50,  // Cell header
    0x00,
    0x00,
    0x00,
    0x00,  // Seqno
};

const uint8_t data_tail[] = {
    0x40  // zero bit + padding
};

bool pubkey_to_hash(const uint8_t public_key[static PUBKEY_LEN], const uint32_t subwallet_id, const bool is_v3r2, uint8_t *out, size_t out_len) {
    if (out_len != HASH_LEN) {
        return false;
    }

    uint8_t inner[HASH_LEN] = {0};
    cx_sha256_t state;

    uint8_t subwallet_buf[4];
    subwallet_buf[0] = (subwallet_id >> 24) & 0xff;
    subwallet_buf[1] = (subwallet_id >> 16) & 0xff;
    subwallet_buf[2] = (subwallet_id >> 8) & 0xff;
    subwallet_buf[3] = subwallet_id & 0xff;

    // Hash init data cell bits
    SAFE(cx_sha256_init_no_throw(&state));
    if (is_v3r2) {
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, v3r2_data_header, sizeof(v3r2_data_header), NULL, 0));
    } else {
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, data_header, sizeof(data_header), NULL, 0));
    }
    SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, subwallet_buf, sizeof(subwallet_buf), NULL, 0));
    if (is_v3r2) {
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, CX_LAST, public_key, PUBKEY_LEN, inner, sizeof(inner)));
    } else {
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, public_key, PUBKEY_LEN, NULL, 0));
        SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                            CX_LAST,
                            data_tail,
                            sizeof(data_tail),
                            inner,
                            sizeof(inner)));
    }

    // Hash root
    SAFE(cx_sha256_init_no_throw(&state));
    if (is_v3r2) {
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, v3r2_root_header, sizeof(v3r2_root_header), NULL, 0));
    } else {
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, root_header, sizeof(root_header), NULL, 0));
    }
    SAFE(cx_hash_no_throw((cx_hash_t *) &state, CX_LAST, inner, sizeof(inner), out, out_len));

    return true;
}

bool address_from_pubkey(const uint8_t public_key[static PUBKEY_LEN],
                         const uint8_t chain,
                         const bool bounceable,
                         const bool testOnly,
                         const uint32_t subwallet_id,
                         const bool is_v3r2,
                         uint8_t *out,
                         size_t out_len) {
    if (out_len < ADDRESS_LEN) {
        return false;
    }

    uint8_t hash[HASH_LEN] = {0};

    if (!pubkey_to_hash(public_key, subwallet_id, is_v3r2, hash, sizeof(hash))) {
        return false;
    }

    // Convert to friendly
    address_to_friendly(chain, hash, bounceable, testOnly, out, out_len);

    return true;
}
