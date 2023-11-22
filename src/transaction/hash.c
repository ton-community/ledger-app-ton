#include <stdbool.h>
#include <string.h>  // memmove

#include "cx.h"

#include "hash.h"

#include "../common/cell.h"
#include "../common/bits.h"
#include "../constants.h"

bool hash_tx(transaction_ctx_t *ctx) {
    BitString_t bits;
    struct CellRef_t payload_ref;
    struct CellRef_t state_init_ref;

    //
    // Internal Message
    //

    struct CellRef_t internalMessageRef;
    BitString_init(&bits);
    BitString_storeBit(&bits, 0);                                // tag
    BitString_storeBit(&bits, 1);                                // ihr_disabled
    BitString_storeBit(&bits, ctx->transaction.bounce ? 1 : 0);  // bounce
    BitString_storeBit(&bits, 0);                                // bounced
    BitString_storeAddressNull(&bits);                           // from
    BitString_storeAddress(&bits, ctx->transaction.to.chain, ctx->transaction.to.hash);  // to
    // amount
    BitString_storeCoinsBuf(&bits, ctx->transaction.value_buf, ctx->transaction.value_len);
    BitString_storeBit(&bits, 0);       // Currency collection (not supported)
    BitString_storeCoins(&bits, 0);     // ihr_fees
    BitString_storeCoins(&bits, 0);     // fwd_fees
    BitString_storeUint(&bits, 0, 64);  // CreatedLT
    BitString_storeUint(&bits, 0, 32);  // CreatedAt

    // Refs
    if (ctx->transaction.has_payload && ctx->transaction.has_state_init) {
        BitString_storeBit(&bits, 1);  // state-init
        BitString_storeBit(&bits, 1);  // state-init ref
        BitString_storeBit(&bits, 1);  // body in ref

        // Create refs
        payload_ref.max_depth = ctx->transaction.payload.max_depth;
        memmove(payload_ref.hash, ctx->transaction.payload.hash, HASH_LEN);
        state_init_ref.max_depth = ctx->transaction.state_init.max_depth;
        memmove(state_init_ref.hash, ctx->transaction.state_init.hash, HASH_LEN);

        // Hash cell
        struct CellRef_t internalMessageRefs[2] = {state_init_ref, payload_ref};
        if (!hash_Cell(&bits, internalMessageRefs, 2, &internalMessageRef)) {
            return false;
        }
    } else if (ctx->transaction.has_payload) {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 1);  // body in ref

        // Create ref
        payload_ref.max_depth = ctx->transaction.payload.max_depth;
        memmove(payload_ref.hash, ctx->transaction.payload.hash, HASH_LEN);

        // Hash cell
        struct CellRef_t internalMessageRefs[1] = {payload_ref};
        if (!hash_Cell(&bits, internalMessageRefs, 1, &internalMessageRef)) {
            return false;
        }
    } else if (ctx->transaction.has_state_init) {
        BitString_storeBit(&bits, 1);  // no state-init
        BitString_storeBit(&bits, 1);  // state-init ref
        BitString_storeBit(&bits, 0);  // body inline

        // Create ref
        state_init_ref.max_depth = ctx->transaction.state_init.max_depth;
        memmove(state_init_ref.hash, ctx->transaction.state_init.hash, HASH_LEN);

        // Hash cell
        struct CellRef_t internalMessageRefs[1] = {state_init_ref};
        if (!hash_Cell(&bits, internalMessageRefs, 1, &internalMessageRef)) {
            return false;
        }
    } else {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 0);  // body inline

        // Hash cell
        if (!hash_Cell(&bits, NULL, 0, &internalMessageRef)) {
            return false;
        }
    }

    //
    // Order
    //

    struct CellRef_t orderRef;
    BitString_init(&bits);
    BitString_storeUint(&bits, 698983191, 32);                  // Wallet ID
    BitString_storeUint(&bits, ctx->transaction.timeout, 32);   // Timeout
    BitString_storeUint(&bits, ctx->transaction.seqno, 32);     // Seqno
    BitString_storeUint(&bits, 0, 8);                           // Simple order
    BitString_storeUint(&bits, ctx->transaction.send_mode, 8);  // Send Mode
    struct CellRef_t orderRefs[1] = {internalMessageRef};
    if (!hash_Cell(&bits, orderRefs, 1, &orderRef)) {
        return false;
    }

    // Result
    memmove(ctx->m_hash, orderRef.hash, HASH_LEN);

    return true;
}