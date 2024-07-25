#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cx.h"

#include "proof_deserialize.h"

#include "../common/buffer.h"
#include "../common/bip32_check.h"
#include "../types.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../crypto.h"
#include "../address.h"
#include "../apdu/params.h"

static const uint8_t TON_PROOF_ITEM_STR[] = "ton-proof-item-v2/";
static const uint8_t TON_CONNECT_STR[] = "\xff\xffton-connect";

#define SAFE(RES)                 \
    if ((RES) != CX_OK) {         \
        io_send_sw(SW_BAD_STATE); \
        return false;             \
    }

bool deserialize_proof(buffer_t *cdata, uint8_t flags) {
    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        io_send_sw(SW_WRONG_DATA_LENGTH);
        return false;
    }

    if (!check_global_bip32_path()) {
        io_send_sw(SW_BAD_BIP32_PATH);
        return false;
    }

    if (crypto_derive_public_key(G_context.bip32_path,
                                 G_context.bip32_path_len,
                                 G_context.proof_info.raw_public_key) < 0) {
        io_send_sw(SW_BAD_STATE);
        return false;
    }

    if (flags & P2_ADDR_FLAG_WALLET_SPECIFIERS) {
        if (!buffer_read_bool(cdata, &G_context.proof_info.is_v3r2) ||
            !buffer_read_u32(cdata, &G_context.proof_info.subwallet_id, BE)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }
    } else {
        G_context.proof_info.subwallet_id = 698983191;
        G_context.proof_info.is_v3r2 = false;
    }

    if (!pubkey_to_hash(G_context.proof_info.raw_public_key,
                        G_context.proof_info.subwallet_id,
                        G_context.proof_info.is_v3r2,
                        G_context.proof_info.address_hash,
                        sizeof(G_context.proof_info.address_hash))) {
        io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
        return false;
    }

    G_context.proof_info.workchain = (flags & P2_ADDR_FLAG_MASTERCHAIN) ? -1 : 0;

    if (!buffer_read_u8(cdata, &G_context.proof_info.domain_len)) {
        io_send_sw(SW_WRONG_DATA_LENGTH);
        return false;
    }

    if (G_context.proof_info.domain_len > sizeof(G_context.proof_info.domain)) {
        io_send_sw(SW_REQUEST_TOO_LONG);
        return false;
    }

    if (!buffer_read_buffer(cdata, G_context.proof_info.domain, G_context.proof_info.domain_len)) {
        io_send_sw(SW_WRONG_DATA_LENGTH);
        return false;
    }

    uint64_t timestamp;

    if (!buffer_read_u64(cdata, &timestamp, BE)) {
        io_send_sw(SW_WRONG_DATA_LENGTH);
        return false;
    }

    size_t payload_len = buffer_remaining(cdata);
    uint8_t payload[128];

    if (payload_len > sizeof(payload)) {
        io_send_sw(SW_REQUEST_TOO_LONG);
        return false;
    }

    if (!buffer_move(cdata, payload, payload_len)) {
        // impossible
        io_send_sw(SW_WRONG_DATA_LENGTH);
        return false;
    }

    cx_sha256_t state;
    SAFE(cx_sha256_init_no_throw(&state));

    // sizeof - 1 because const strings are null terminated
    SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                          0,
                          TON_PROOF_ITEM_STR,
                          sizeof(TON_PROOF_ITEM_STR) - 1,
                          NULL,
                          0));

    for (int i = 3; i >= 0; i--) {
        uint8_t wc_part = (G_context.proof_info.workchain >> (i * 8)) & 0xff;
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, &wc_part, 1, NULL, 0));
    }
    SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                          0,
                          G_context.proof_info.address_hash,
                          sizeof(G_context.proof_info.address_hash),
                          NULL,
                          0));

    for (int i = 0; i < 4; i++) {
        uint8_t domain_len_part = (G_context.proof_info.domain_len >> (i * 8)) & 0xff;
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, &domain_len_part, 1, NULL, 0));
    }
    SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                          0,
                          G_context.proof_info.domain,
                          G_context.proof_info.domain_len,
                          NULL,
                          0));

    for (int i = 0; i < 8; i++) {
        uint8_t ts_part = (timestamp >> (i * 8)) & 0xff;
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, &ts_part, 1, NULL, 0));
    }

    uint8_t inner[HASH_LEN];

    SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                          CX_LAST,
                          payload,
                          payload_len,
                          inner,
                          sizeof(inner)));

    SAFE(cx_sha256_init_no_throw(&state));

    // sizeof - 1 because const strings are null terminated
    SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                          0,
                          TON_CONNECT_STR,
                          sizeof(TON_CONNECT_STR) - 1,
                          NULL,
                          0));

    SAFE(cx_hash_no_throw((cx_hash_t *) &state,
                          CX_LAST,
                          inner,
                          sizeof(inner),
                          G_context.proof_info.hash,
                          sizeof(G_context.proof_info.hash)));

    return true;
}
