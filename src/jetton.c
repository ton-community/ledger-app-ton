// Todo: initial author copyright ?
// SPDX-FileCopyrightText: 2025 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

/*
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
 */

#ifdef HAVE_HARDCODED_JETTONS

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>

#include "common/bits.h"
#include "common/cell.h"
#include "common/types.h"
#include "address.h"
#include "constants.h"

#include "jetton.h"

#define SAFE(RES)     \
    if (!RES) {       \
        return false; \
    }

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
    uint8_t master_hash[HASH_LEN];
    char name[JETTON_NAME_MAX_SIZE]; /* Fixme: should be ticker max len ? i.e. 16 ? */
    CellRef_t code;
    uint8_t master_workchain;
    uint8_t decimals;
    uint8_t state_assembler_idx;
} jetton_t;

static const jetton_t jettons[] = {
    {
        .master_hash = {0xb1, 0x13, 0xa9, 0x94, 0xb5, 0x02, 0x4a, 0x16, 0x71, 0x9f, 0x69,
                        0x13, 0x93, 0x28, 0xeb, 0x75, 0x95, 0x96, 0xc3, 0x8a, 0x25, 0xf5,
                        0x90, 0x28, 0xb1, 0x46, 0xfe, 0xcd, 0xc3, 0x62, 0x1d, 0xfe},
        .name = "USDT",
        .code =
            {
                .hash = {0x89, 0x46, 0x8f, 0x02, 0xc7, 0x8e, 0x57, 0x08, 0x02, 0xe3, 0x99,
                         0x79, 0xc8, 0x51, 0x6f, 0xc3, 0x8d, 0xf0, 0x7e, 0xa7, 0x6a, 0x48,
                         0x35, 0x7e, 0x05, 0x36, 0xf2, 0xba, 0x7b, 0x3e, 0xe3, 0x7b},
                .max_depth = 0,
            },
        .master_workchain = 0,
        .decimals = 6,
        .state_assembler_idx = 0,
    },
    {
        .master_hash = {0x2f, 0x95, 0x61, 0x43, 0xc4, 0x61, 0x76, 0x95, 0x79, 0xba, 0xef,
                        0x2e, 0x32, 0xcc, 0x2d, 0x7b, 0xc1, 0x82, 0x83, 0xf4, 0xd,  0x20,
                        0xbb, 0x3,  0xe4, 0x32, 0xcd, 0x60, 0x3a, 0xc3, 0x3f, 0xfc},
        .name = "NOT",
        .code =
            {
                .hash = {0x8d, 0x28, 0xea, 0x42, 0x1b, 0x77, 0xe8, 0x5,  0xfe, 0xa5, 0x2a,
                         0xcf, 0x33, 0x52, 0x96, 0x49, 0x9f, 0x3,  0xae, 0xc8, 0xe9, 0xfd,
                         0x21, 0xdd, 0xb5, 0xf2, 0x56, 0x4a, 0xa6, 0x5c, 0x48, 0xde},
                .max_depth = 0,
            },
        .master_workchain = 0,
        .decimals = 9,
        .state_assembler_idx = 0,
    },
    {
        .master_hash = {0xbd, 0xf3, 0xfa, 0x80, 0x98, 0xd1, 0x29, 0xb5, 0x4b, 0x4f, 0x73,
                        0xb5, 0xba, 0xc5, 0xd1, 0xe1, 0xfd, 0x91, 0xeb, 0x5,  0x41, 0x69,
                        0xc3, 0x91, 0x6d, 0xfc, 0x8c, 0xcd, 0x53, 0x6d, 0x10, 0x0},
        .name = "tsTON",
        .code =
            {
                .hash = {0x8,  0x96, 0x21, 0xdd, 0x77, 0xe0, 0xa9, 0xa4, 0xae, 0x44, 0x27,
                         0x9e, 0x59, 0x2f, 0x13, 0xd6, 0xab, 0x5c, 0x55, 0xde, 0xcd, 0x60,
                         0xcc, 0x9e, 0xc3, 0x6b, 0x7c, 0x2a, 0xe4, 0xd,  0x15, 0x95},
                .max_depth = 0,
            },
        .master_workchain = 0,
        .decimals = 9,
        .state_assembler_idx = 1,
    },
    {
        .master_hash = {0x74, 0x4a, 0x8c, 0x6e, 0x18, 0x3c, 0x79, 0xaa, 0x35, 0x6d, 0xd0,
                        0xff, 0xdb, 0x3c, 0x80, 0x85, 0x79, 0x67, 0x45, 0x2c, 0x19, 0x95,
                        0xa2, 0x91, 0xe1, 0x8e, 0x7,  0xec, 0xd2, 0xaf, 0xb0, 0xb1},
        .name = "wsTON",
        .code =
            {
                .hash = {0xda, 0x4d, 0x2,  0xf6, 0xb4, 0xea, 0x63, 0x4,  0x16, 0xc3, 0x34,
                         0x9a, 0xfc, 0x75, 0xc8, 0x2,  0x15, 0x1b, 0xb1, 0x6f, 0xd6, 0x47,
                         0xae, 0x9f, 0x73, 0xbd, 0xe0, 0x17, 0x73, 0x55, 0x72, 0x31},
                .max_depth = 7,
            },
        .master_workchain = 0,
        .decimals = 9,
        .state_assembler_idx = 2,
    },
    {
        .master_hash = {0xcf, 0x76, 0xaf, 0x31, 0x8c, 0x8,  0x72, 0xb5, 0x8a, 0x9f, 0x19,
                        0x25, 0xfc, 0x29, 0xc1, 0x56, 0x21, 0x17, 0x82, 0xb9, 0xfb, 0x1,
                        0xf5, 0x67, 0x60, 0xd2, 0x92, 0xe5, 0x61, 0x23, 0xbf, 0x87},
        .name = "hTON",
        .code =
            {
                .hash = {0x50, 0xb9, 0x17, 0xd9, 0xfd, 0x5b, 0x95, 0x76, 0x9,  0x93, 0x28,
                         0x76, 0x77, 0xd0, 0x48, 0x12, 0xa1, 0x5a, 0xa5, 0x5e, 0x6b, 0x71,
                         0xd0, 0xf9, 0x22, 0x6e, 0xcd, 0x7c, 0x20, 0xc3, 0xd8, 0x2a},
                .max_depth = 0,
            },
        .master_workchain = 0,
        .decimals = 9,
        .state_assembler_idx = 3,
    },
    {
        .master_hash = {0xcd, 0x87, 0x2f, 0xa7, 0xc5, 0x81, 0x60, 0x52, 0xac, 0xdf, 0x53,
                        0x32, 0x26, 0x4,  0x43, 0xfa, 0xec, 0x9a, 0xac, 0xc8, 0xc2, 0x1c,
                        0xca, 0x4d, 0x92, 0xe7, 0xf4, 0x70, 0x34, 0xd1, 0x18, 0x92},
        .name = "stTON",
        .code =
            {
                .hash = {0x2d, 0x79, 0x72, 0xf7, 0x12, 0xfc, 0x39, 0x8b, 0xc3, 0x2f, 0x67,
                         0x96, 0x98, 0xa,  0xd7, 0x11, 0x90, 0x74, 0x34, 0xa3, 0x11, 0xb5,
                         0x7f, 0xe7, 0x2d, 0x20, 0xf7, 0x51, 0xb,  0x21, 0x23, 0xe6},
                .max_depth = 6,
            },
        .master_workchain = 0,
        .decimals = 9,
        .state_assembler_idx = 4,
    },
    {
        .master_hash = {0xaa, 0xb,  0xa1, 0x21, 0x44, 0x9f, 0xed, 0xa5, 0x69, 0xe0, 0x2b,
                        0x12, 0xfa, 0x75, 0x5d, 0x24, 0xe8, 0x34, 0xa7, 0x45, 0x4a, 0xec,
                        0xf4, 0x64, 0x95, 0x90, 0xb6, 0xdf, 0x74, 0x2a, 0xac, 0x8f},
        .name = "STAKED",
        .code =
            {
                .hash = {0x8,  0x96, 0x21, 0xdd, 0x77, 0xe0, 0xa9, 0xa4, 0xae, 0x44, 0x27,
                         0x9e, 0x59, 0x2f, 0x13, 0xd6, 0xab, 0x5c, 0x55, 0xde, 0xcd, 0x60,
                         0xcc, 0x9e, 0xc3, 0x6b, 0x7c, 0x2a, 0xe4, 0xd,  0x15, 0x95},
                .max_depth = 0,
            },
        .master_workchain = 0,
        .decimals = 9,
        .state_assembler_idx = 1,
    },
};

static void assemble_usdt_state(CellRef_t *out,
                                const CellRef_t *code,
                                const address_t *owner,
                                const address_t *master) {
    BitString_t bits;
    CellRef_t refs[2];
    refs[0] = *code;

    BitString_init(&bits);
    BitString_storeUint(&bits, 0, 4);
    BitString_storeCoins(&bits, 0);
    BitString_storeAddress(&bits, owner->chain, owner->hash);
    BitString_storeAddress(&bits, master->chain, master->hash);
    hash_Cell(&bits, NULL, 0, &refs[1]);

    BitString_init(&bits);
    BitString_storeUint(&bits, 0b00110, 5);
    hash_Cell(&bits, refs, 2, out);
}

static void assemble_tston_state(CellRef_t *out,
                                 const CellRef_t *code,
                                 const address_t *owner,
                                 const address_t *master) {
    BitString_t bits;
    CellRef_t refs[2];
    refs[0] = *code;

    BitString_init(&bits);
    BitString_storeCoins(&bits, 0);
    BitString_storeAddress(&bits, owner->chain, owner->hash);
    BitString_storeAddress(&bits, master->chain, master->hash);
    BitString_storeCoins(&bits, 0);
    BitString_storeUint(&bits, 0, 48);
    hash_Cell(&bits, refs, 1, &refs[1]);

    BitString_init(&bits);
    BitString_storeUint(&bits, 0b00110, 5);
    hash_Cell(&bits, refs, 2, out);
}

static void assemble_wston_state(CellRef_t *out,
                                 const CellRef_t *code,
                                 const address_t *owner,
                                 const address_t *master) {
    BitString_t bits;
    CellRef_t refs[2];
    refs[0] = *code;

    BitString_init(&bits);
    BitString_storeCoins(&bits, 0);
    BitString_storeAddress(&bits, owner->chain, owner->hash);
    BitString_storeAddress(&bits, master->chain, master->hash);
    BitString_storeBit(&bits, 0);
    hash_Cell(&bits, refs, 1, &refs[1]);

    BitString_init(&bits);
    BitString_storeUint(&bits, 0b00110, 5);
    hash_Cell(&bits, refs, 2, out);
}

static void assemble_hton_state(CellRef_t *out,
                                const CellRef_t *code,
                                const address_t *owner,
                                const address_t *master) {
    BitString_t bits;
    CellRef_t refs[2];
    refs[0] = *code;

    BitString_init(&bits);
    BitString_storeAddress(&bits, owner->chain, owner->hash);
    BitString_storeAddress(&bits, master->chain, master->hash);
    BitString_storeCoins(&bits, 0);
    BitString_storeBit(&bits, 0);
    BitString_storeCoins(&bits, 0);
    hash_Cell(&bits, NULL, 0, &refs[1]);

    BitString_init(&bits);
    BitString_storeUint(&bits, 0b00110, 5);
    hash_Cell(&bits, refs, 2, out);
}

static void assemble_stton_state(CellRef_t *out,
                                 const CellRef_t *code,
                                 const address_t *owner,
                                 const address_t *master) {
    BitString_t bits;
    CellRef_t refs[2];
    refs[0] = *code;

    BitString_init(&bits);
    BitString_storeCoins(&bits, 0);
    BitString_storeAddress(&bits, owner->chain, owner->hash);
    BitString_storeAddress(&bits, master->chain, master->hash);
    hash_Cell(&bits, refs, 1, &refs[1]);

    BitString_init(&bits);
    BitString_storeUint(&bits, 0b00110, 5);
    hash_Cell(&bits, refs, 2, out);
}

static bool jetton_state_assembler_dispatcher(CellRef_t *out,
                                              const CellRef_t *code,
                                              const address_t *owner,
                                              const address_t *master,
                                              uint8_t state_assembler_idx) {
    switch (state_assembler_idx) {
        case 0:
            assemble_usdt_state(out, code, owner, master);
            return true;
        case 1:
            assemble_tston_state(out, code, owner, master);
            return true;
        case 2:
            assemble_wston_state(out, code, owner, master);
            return true;
        case 3:
            assemble_hton_state(out, code, owner, master);
            return true;
        case 4:
            assemble_stton_state(out, code, owner, master);
            return true;
    }
    return false;
}

static bool jetton_get_id_by_name(const char *name, uint8_t *id) {
    for (uint8_t idx = 0U; idx < ARRAY_SIZE(jettons); idx++) {
        if (strncmp(jettons[idx].name, name, JETTON_NAME_MAX_SIZE) == 0) {
            *id = idx;
            return true;
        }
    }

    return false;
}

bool jetton_get_wallet_address(size_t jetton_id, const address_t *owner, address_t *jetton_wallet) {
    CellRef_t state_init;
    address_t master;

    if (jetton_id > ARRAY_SIZE(jettons)) {
        return false;
    }

    if (owner == NULL) {
        return false;
    }

    if (jetton_wallet == NULL) {
        return false;
    }

    explicit_bzero(jetton_wallet, sizeof(address_t));

    memcpy(master.hash, jettons[jetton_id].master_hash, HASH_LEN);
    master.chain = jettons[jetton_id].master_workchain;

    SAFE(jetton_state_assembler_dispatcher(&state_init,
                                           &jettons[jetton_id].code,
                                           owner,
                                           &master,
                                           jettons[jetton_id].state_assembler_idx));

    jetton_wallet->chain = master.chain;
    memcpy(jetton_wallet->hash, state_init.hash, HASH_LEN);

    return true;
}

const char *jetton_get_name(size_t jetton_id) {
    if (jetton_id > ARRAY_SIZE(jettons)) {
        return "";
    }

    return jettons[jetton_id].name;
}

uint8_t jetton_get_decimals(size_t jetton_id) {
    if (jetton_id > ARRAY_SIZE(jettons)) {
        return 0U;
    }

    return jettons[jetton_id].decimals;
}

bool jetton_get_wallet_address_by_name(const char *jetton_ticker,
                                       const address_t *owner,
                                       address_t *jetton_wallet) {
    uint8_t jetton_id;

    if (jetton_ticker == NULL) {
        return false;
    }

    if (!jetton_get_id_by_name(jetton_ticker, &jetton_id)) {
        return false;
    }

    return jetton_get_wallet_address(jetton_id, owner, jetton_wallet);
}

#endif /* HAVE_HARDCODED_JETTONS */
