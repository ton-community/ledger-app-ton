#include <string.h>
#include <stdio.h>

#include "transaction_hints.h"

#include "common/mybuffer.h"
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
#include "../jetton.h"

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
                address_t owner;
                address_t jetton_wallet;
                uint8_t pubkey[32];

                SAFE(buffer_read_u16(&buf, &jetton_id, BE));
                SAFE(buffer_read_u8(&buf, &owner.chain));

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

                SAFE(jetton_get_wallet_address(jetton_id, &owner, &jetton_wallet));

                if (memcmp(jetton_wallet.hash, G_context.tx_info.transaction.to.hash, HASH_LEN) != 0) {
                    return false;
                }

                uint8_t amount_size;
                uint8_t amount_buf[MAX_VALUE_BYTES_LEN];
                SAFE(buffer_read_varuint(&buf, &amount_size, amount_buf, MAX_VALUE_BYTES_LEN));
                BitString_storeCoinsBuf(&bits, amount_buf, amount_size);

                add_hint_amount(&tx->hints,
                                "Jetton amount",
                                jetton_get_name(jetton_id),
                                amount_buf,
                                amount_size,
                                jetton_get_decimals(jetton_id));
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
