#include <string.h>
#include <stdio.h>

#include "transaction_hints.h"

#include "../common/buffer.h"
#include "../common/base64.h"
#include "../common/format_bigint.h"
#include "../common/format_address.h"
#include "../common/encoding.h"
#include "../constants.h"
#include "deserialize.h"
#include "../common/hints.h"
#include "../common/bits.h"
#include "../common/cell.h"
#include "../globals.h"
#include "../crypto.h"
#include "../address.h"

#define SAFE(RES)     \
    if (!RES) {       \
        return false; \
    }

#define CHECK_END()               \
    if (buf.offset != buf.size) { \
        return false;             \
    }

static const uint8_t dns_key_wallet[32] = {
    0xe8, 0xd4, 0x40, 0x50, 0x87, 0x3d, 0xba, 0x86, 0x5a, 0xa7, 0xc1, 0x70, 0xab, 0x4c, 0xce, 0x64,
    0xd9, 0x08, 0x39, 0xa3, 0x4d, 0xcf, 0xd6, 0xcf, 0x71, 0xd1, 0x4e, 0x02, 0x05, 0x44, 0x3b, 0x1b};

#ifdef HAVE_HARDCODED_JETTONS

static void assemble_usdt_state(CellRef_t* out,
                                const CellRef_t* code,
                                address_t* owner,
                                address_t* master) {
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

static void assemble_tston_state(CellRef_t* out,
                                 const CellRef_t* code,
                                 address_t* owner,
                                 address_t* master) {
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

static void assemble_wston_state(CellRef_t* out,
                                 const CellRef_t* code,
                                 address_t* owner,
                                 address_t* master) {
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

static void assemble_hton_state(CellRef_t* out,
                                const CellRef_t* code,
                                address_t* owner,
                                address_t* master) {
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

static void assemble_stton_state(CellRef_t* out,
                                 const CellRef_t* code,
                                 address_t* owner,
                                 address_t* master) {
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

static bool jetton_state_assembler_dispatcher(CellRef_t* out,
                                              const CellRef_t* code,
                                              address_t* owner,
                                              address_t* master,
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

typedef struct {
    uint8_t master_hash[32];
    char name[8];
    CellRef_t code;
    uint8_t master_workchain;
    uint8_t decimals;
    uint8_t state_assembler_idx;
} jetton_t;

static const jetton_t jettons[] = {
    {
        .master_hash = {0xb1, 0x13, 0xa9, 0x94, 0xb5, 0x2,  0x4a, 0x16, 0x71, 0x9f, 0x69,
                        0x13, 0x93, 0x28, 0xeb, 0x75, 0x95, 0x96, 0xc3, 0x8a, 0x25, 0xf5,
                        0x90, 0x28, 0xb1, 0x46, 0xfe, 0xcd, 0xc3, 0x62, 0x1d, 0xfe},
        .name = "USDT",
        .code =
            {
                .hash = {0x89, 0x46, 0x8f, 0x2,  0xc7, 0x8e, 0x57, 0x8,  0x2,  0xe3, 0x99,
                         0x79, 0xc8, 0x51, 0x6f, 0xc3, 0x8d, 0xf0, 0x7e, 0xa7, 0x6a, 0x48,
                         0x35, 0x7e, 0x5,  0x36, 0xf2, 0xba, 0x7b, 0x3e, 0xe3, 0x7b},
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

#endif

bool process_hints(transaction_t* tx) {
    // Default title
    snprintf(tx->title, sizeof(tx->title), "Transaction");
    snprintf(tx->action, sizeof(tx->action), "send TON");
    snprintf(tx->recipient, sizeof(tx->recipient), "To");

    // No payload
    if (!tx->has_payload) {
        snprintf(tx->title, sizeof(tx->title), "Transfer");
        tx->is_blind = false;
        return true;
    }
    // No hints
    if (!tx->has_hints) {
        tx->is_blind = true;
        return true;
    }

    // Default state
    tx->is_blind = true;
    CellRef_t cell;
    BitString_t bits;
    bool hasCell = false;
    bool tmp = false;
    buffer_t buf = {.ptr = tx->hints_data, .size = tx->hints_len, .offset = 0};

    //
    // Comment
    //

    if (tx->hints_type == TRANSACTION_COMMENT) {
        // Max size of a comment is 120 symbols
        if (tx->hints_len > MAX_MEMO_LEN) {
            return false;
        }

        // Check ASCII
        if (!check_ascii(tx->hints_data, tx->hints_len)) {
            return false;
        }

        // Build cell
        BitString_init(&bits);
        BitString_storeUint(&bits, 0, 32);
        BitString_storeBuffer(&bits, tx->hints_data, tx->hints_len);
        SAFE(hash_Cell(&bits, NULL, 0, &cell));
        hasCell = true;

        // Change title of operation
        snprintf(tx->title, sizeof(tx->title), "Transfer");

        // Add code hints
        add_hint_text(&tx->hints, "Comment", (char*) tx->hints_data, tx->hints_len);
    }

    if (tx->hints_type == TRANSACTION_TRANSFER_JETTON ||
        tx->hints_type == TRANSACTION_TRANSFER_NFT) {
        int ref_count = 0;
        CellRef_t refs[2] = {0};
        uint8_t flags;

        BitString_init(&bits);
        BitString_storeUint(&bits,
                            tx->hints_type == TRANSACTION_TRANSFER_JETTON ? 0x0f8a7ea5 : 0x5fcc3d14,
                            32);

        SAFE(buffer_read_u8(&buf, &flags));
        bool has_query_id = flags & 1;
        if (has_query_id) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        if (tx->hints_type == TRANSACTION_TRANSFER_JETTON) {
            bool has_jetton_id = (flags & 2) != 0;
            if (has_jetton_id) {
#ifdef HAVE_HARDCODED_JETTONS
                uint16_t jetton_id;
                SAFE(buffer_read_u16(&buf, &jetton_id, BE));

                if (jetton_id > sizeof(jettons) / sizeof(jettons[0])) {
                    return false;
                }

                CellRef_t state_init;

                address_t owner;

                SAFE(buffer_read_u8(&buf, &owner.chain));

                uint8_t pubkey[32];

                if (crypto_derive_public_key(G_context.bip32_path,
                                             G_context.bip32_path_len,
                                             pubkey) < 0) {
                    return false;
                }

                SAFE(pubkey_to_hash(pubkey,
                                    G_context.tx_info.transaction.subwallet_id,
                                    !G_context.tx_info.transaction.include_wallet_op,
                                    owner.hash,
                                    sizeof(owner.hash)));

                address_t master;
                memcpy(master.hash, jettons[jetton_id].master_hash, 32);
                master.chain = jettons[jetton_id].master_workchain;

                SAFE(jetton_state_assembler_dispatcher(&state_init,
                                                       &jettons[jetton_id].code,
                                                       &owner,
                                                       &master,
                                                       jettons[jetton_id].state_assembler_idx));

                if (memcmp(state_init.hash, G_context.tx_info.transaction.to.hash, 32) != 0) {
                    return false;
                }

                uint8_t amount_size;
                uint8_t amount_buf[MAX_VALUE_BYTES_LEN];
                SAFE(buffer_read_varuint(&buf, &amount_size, amount_buf, MAX_VALUE_BYTES_LEN));
                BitString_storeCoinsBuf(&bits, amount_buf, amount_size);

                add_hint_amount(&tx->hints,
                                "Jetton amount",
                                jettons[jetton_id].name,
                                amount_buf,
                                amount_size,
                                jettons[jetton_id].decimals);
#else
                return false;
#endif
            } else {
                uint8_t amount_size;
                uint8_t amount_buf[MAX_VALUE_BYTES_LEN];
                SAFE(buffer_read_varuint(&buf, &amount_size, amount_buf, MAX_VALUE_BYTES_LEN));
                BitString_storeCoinsBuf(&bits, amount_buf, amount_size);

                add_hint_amount(&tx->hints, "Jetton units", "", amount_buf, amount_size, 0);
            }
        }

        address_t destination;
        SAFE(buffer_read_address(&buf, &destination));
        BitString_storeAddress(&bits, destination.chain, destination.hash);

        add_hint_address(
            &tx->hints,
            tx->hints_type == TRANSACTION_TRANSFER_JETTON ? "Send jetton to" : "New owner",
            destination,
            false);

        address_t response;
        SAFE(buffer_read_address(&buf, &response));
        BitString_storeAddress(&bits, response.chain, response.hash);

        if (N_storage.expert_mode) {
            add_hint_address(&tx->hints, "Send excess to", response, false);
        }

        // custom payload
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            SAFE(buffer_read_cell_ref(&buf, &refs[ref_count]));

            if (N_storage.expert_mode) {
                add_hint_hash(&tx->hints, "Custom payload", refs[ref_count].hash);
            }

            BitString_storeBit(&bits, 1);
            ref_count++;
        } else {
            BitString_storeBit(&bits, 0);
        }

        uint8_t fwd_amount_size;
        uint8_t fwd_amount_buf[MAX_VALUE_BYTES_LEN];
        SAFE(buffer_read_varuint(&buf, &fwd_amount_size, fwd_amount_buf, MAX_VALUE_BYTES_LEN));
        BitString_storeCoinsBuf(&bits, fwd_amount_buf, fwd_amount_size);

        if (N_storage.expert_mode) {
            add_hint_amount(&tx->hints,
                            "Forward amount",
                            "TON",
                            fwd_amount_buf,
                            fwd_amount_size,
                            EXPONENT_SMALLEST_UNIT);
        }

        // forward payload
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            SAFE(buffer_read_cell_ref(&buf, &refs[ref_count]));

            if (N_storage.expert_mode) {
                add_hint_hash(&tx->hints, "Forward payload", refs[ref_count].hash);
            }

            BitString_storeBit(&bits, 1);
            ref_count++;
        } else {
            BitString_storeBit(&bits, 0);
        }

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, refs, ref_count, &cell));
        hasCell = true;

        // Operation
        snprintf(
            tx->title,
            sizeof(tx->title),
            tx->hints_type == TRANSACTION_TRANSFER_JETTON ? "Transfer jetton" : "Transfer NFT");
        snprintf(
            tx->action,
            sizeof(tx->action),
            tx->hints_type == TRANSACTION_TRANSFER_JETTON ? "transfer jetton" : "transfer NFT");
        snprintf(tx->recipient,
                 sizeof(tx->recipient),
                 tx->hints_type == TRANSACTION_TRANSFER_JETTON ? "Jetton wallet" : "NFT Address");
    }

    if (tx->hints_type == TRANSACTION_BURN_JETTON) {
        int ref_count = 0;
        CellRef_t refs[1] = {0};

        BitString_init(&bits);
        BitString_storeUint(&bits, 0x595f07bc, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        uint8_t amount_size;
        uint8_t amount_buf[MAX_VALUE_BYTES_LEN];
        SAFE(buffer_read_varuint(&buf, &amount_size, amount_buf, MAX_VALUE_BYTES_LEN));
        BitString_storeCoinsBuf(&bits, amount_buf, amount_size);

        add_hint_amount(&tx->hints, "Jetton units", "", amount_buf, amount_size, 0);

        address_t response;
        SAFE(buffer_read_address(&buf, &response));
        BitString_storeAddress(&bits, response.chain, response.hash);

        if (N_storage.expert_mode) {
            add_hint_address(&tx->hints, "Send excess to", response, false);
        }

        // custom payload
        uint8_t type;
        SAFE(buffer_read_u8(&buf, &type));
        if (type == 0x00) {
            BitString_storeBit(&bits, 0);
        } else if (type == 0x01) {
            SAFE(buffer_read_cell_ref(&buf, &refs[ref_count]));

            if (N_storage.expert_mode) {
                add_hint_hash(&tx->hints, "Custom payload", refs[ref_count].hash);
            }

            BitString_storeBit(&bits, 1);
            ref_count++;
        } else if (type == 0x02) {
            uint8_t len;
            SAFE(buffer_read_u8(&buf, &len));

            if (len > MAX_CELL_INLINE_LEN) {
                return false;
            }

            uint8_t data[MAX_CELL_INLINE_LEN];
            SAFE(buffer_read_buffer(&buf, data, len));

            add_hint_hex(&tx->hints, "Custom payload", data, len);

            BitString_t inner_bits;
            BitString_init(&inner_bits);
            BitString_storeBuffer(&inner_bits, data, len);

            hash_Cell(&inner_bits, NULL, 0, &refs[ref_count]);
            ref_count++;

            BitString_storeBit(&bits, 1);
        } else {
            return false;
        }

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, refs, ref_count, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), "Burn jetton");
        snprintf(tx->action, sizeof(tx->action), "burn jetton");
        snprintf(tx->recipient, sizeof(tx->recipient), "Jetton wallet");
    }

    if (tx->hints_type == TRANSACTION_ADD_WHITELIST ||
        tx->hints_type == TRANSACTION_SINGLE_NOMINATOR_CHANGE_VALIDATOR) {
        BitString_init(&bits);
        BitString_storeUint(&bits,
                            tx->hints_type == TRANSACTION_ADD_WHITELIST ? 0x7258a69b : 0x1001,
                            32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        address_t addr;
        SAFE(buffer_read_address(&buf, &addr));
        BitString_storeAddress(&bits, addr.chain, addr.hash);

        add_hint_address(
            &tx->hints,
            tx->hints_type == TRANSACTION_ADD_WHITELIST ? "New whitelist" : "New validator",
            addr,
            false);

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, NULL, 0, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title,
                 sizeof(tx->title),
                 tx->hints_type == TRANSACTION_ADD_WHITELIST ? "Add whitelist" : "Edit validator");
        snprintf(
            tx->action,
            sizeof(tx->action),
            tx->hints_type == TRANSACTION_ADD_WHITELIST ? "add whitelist" : "change validator");
        snprintf(
            tx->recipient,
            sizeof(tx->recipient),
            tx->hints_type == TRANSACTION_ADD_WHITELIST ? "Vesting wallet" : "Single Nominator");
    }

    if (tx->hints_type == TRANSACTION_SINGLE_NOMINATOR_WITHDRAW) {
        BitString_init(&bits);
        BitString_storeUint(&bits, 0x1000, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        uint8_t amount_size;
        uint8_t amount_buf[MAX_VALUE_BYTES_LEN];
        SAFE(buffer_read_varuint(&buf, &amount_size, amount_buf, MAX_VALUE_BYTES_LEN));
        BitString_storeCoinsBuf(&bits, amount_buf, amount_size);

        add_hint_amount(&tx->hints,
                        "Withdraw amount",
                        "TON",
                        amount_buf,
                        amount_size,
                        EXPONENT_SMALLEST_UNIT);

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, NULL, 0, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), "Withdraw stake");
        snprintf(tx->action, sizeof(tx->action), "withdraw from nominator");
        snprintf(tx->recipient, sizeof(tx->recipient), "Single Nominator");
    }

    if (tx->hints_type == TRANSACTION_TONSTAKERS_DEPOSIT) {
        BitString_init(&bits);
        BitString_storeUint(&bits, 0x47d54391, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t app_id;
            SAFE(buffer_read_u64(&buf, &app_id, BE));
            BitString_storeUint(&bits, app_id, 64);
        }

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, NULL, 0, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), "Deposit stake");
        snprintf(tx->action, sizeof(tx->action), "deposit stake");
        snprintf(tx->recipient, sizeof(tx->recipient), "Pool");
    }

    if (tx->hints_type == TRANSACTION_JETTON_DAO_VOTE) {
        BitString_init(&bits);
        BitString_storeUint(&bits, 0x69fb306c, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        address_t voting_address;
        SAFE(buffer_read_address(&buf, &voting_address));
        BitString_storeAddress(&bits, voting_address.chain, voting_address.hash);

        add_hint_address(&tx->hints, "Voting address", voting_address, true);

        uint64_t expiration_date;
        SAFE(buffer_read_u48(&buf, &expiration_date, BE));
        BitString_storeUint(&bits, expiration_date, 48);

        add_hint_number(&tx->hints, "Expiration time", expiration_date);

        // vote
        SAFE(buffer_read_bool(&buf, &tmp));
        BitString_storeBit(&bits, tmp);

        add_hint_bool(&tx->hints, "Vote", tmp);

        // need_confirmation
        SAFE(buffer_read_bool(&buf, &tmp));
        BitString_storeBit(&bits, tmp);

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, NULL, 0, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), "Vote proposal");
        snprintf(tx->action, sizeof(tx->action), "vote for proposal");
        snprintf(tx->recipient, sizeof(tx->recipient), "Jetton wallet");
    }

    if (tx->hints_type == TRANSACTION_CHANGE_DNS_RECORD) {
        int ref_count = 0;
        CellRef_t refs[1] = {0};

        BitString_init(&bits);
        BitString_storeUint(&bits, 0x4eb1f0f9, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        bool has_value;
        SAFE(buffer_read_bool(&buf, &has_value));

        uint8_t type;
        SAFE(buffer_read_u8(&buf, &type));

        if (type == 0x00) {  // wallet
            add_hint_text(&tx->hints, "Type", "Wallet", 6);

            BitString_storeBuffer(&bits, dns_key_wallet, sizeof(dns_key_wallet));

            if (has_value) {
                address_t address;
                SAFE(buffer_read_address(&buf, &address));

                bool has_capabilities;
                SAFE(buffer_read_bool(&buf, &has_capabilities));

                bool is_wallet = false;
                if (has_capabilities) {
                    SAFE(buffer_read_bool(&buf, &is_wallet));
                }

                add_hint_address(&tx->hints, "Wallet address", address, !is_wallet);

                BitString_t inner_bits;
                BitString_init(&inner_bits);

                BitString_storeUint(&inner_bits, 0x9fd3, 16);

                BitString_storeAddress(&inner_bits, address.chain, address.hash);

                BitString_storeUint(&inner_bits, has_capabilities ? 0x01 : 0x00, 8);

                if (has_capabilities) {
                    if (is_wallet) {
                        BitString_storeBit(&inner_bits, 1);
                        BitString_storeUint(&inner_bits, 0x2177, 16);
                    }

                    BitString_storeBit(&inner_bits, 0);
                }

                hash_Cell(&inner_bits, NULL, 0, &refs[ref_count++]);
            }
        } else if (type == 0x01) {  // unknown key
            add_hint_text(&tx->hints, "Type", "Unknown", 7);

            uint8_t key[32];
            SAFE(buffer_read_buffer(&buf, key, sizeof(key)));

            BitString_storeBuffer(&bits, key, sizeof(key));

            add_hint_hash(&tx->hints, "Key", key);

            if (has_value) {
                SAFE(buffer_read_cell_ref(&buf, &refs[ref_count++]));

                add_hint_hash(&tx->hints, "Value", refs[ref_count - 1].hash);
            }
        } else {
            return false;
        }

        if (!has_value) {
            add_hint_bool(&tx->hints, "Delete value", true);
        }

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, refs, ref_count, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), "Change DNS");
        snprintf(tx->action, sizeof(tx->action), "change DNS record");
        snprintf(tx->recipient, sizeof(tx->recipient), "DNS resolver");
    }

    if (tx->hints_type == TRANSACTION_TOKEN_BRIDGE_PAY_SWAP) {
        BitString_init(&bits);
        BitString_storeUint(&bits, 0x8, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        uint8_t swap_id[32];
        SAFE(buffer_read_buffer(&buf, swap_id, sizeof(swap_id)));

        BitString_storeBuffer(&bits, swap_id, sizeof(swap_id));

        add_hint_hash(&tx->hints, "Transfer ID", swap_id);

        CHECK_END();

        // Build cell
        SAFE(hash_Cell(&bits, NULL, 0, &cell));
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), "Bridge tokens");
        snprintf(tx->action, sizeof(tx->action), "bridge tokens");
        snprintf(tx->recipient, sizeof(tx->recipient), "Bridge");
    }

    // Check hash
    if (hasCell) {
        if (memcmp(cell.hash, tx->payload.hash, HASH_LEN) != 0) {
            return false;
        }
        tx->is_blind = false;
    }

    return true;
}
