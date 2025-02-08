#include <stdbool.h>
#include <string.h>  // memmove

#include "cx.h"

#include "hash.h"

#include "../common/cell.h"
#include "../common/bits.h"
#include "../constants.h"

bool hash_tx(transaction_ctx_t *ctx) {
    BitString_t bits;

    struct CellRef_t refs[3];
    int ref_count = 0;

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
    if (transaction_include_extra_currency(&ctx->transaction)) {
        BitString_storeBit(&bits, 1);

        BitString_t ec_bits;
        BitString_init(&ec_bits);
        BitString_storeUint(&ec_bits, 0b10, 2); // hashmap label long
        BitString_storeUint(&ec_bits, 32, 6); // hashmap label length
        BitString_storeUint(&ec_bits, ctx->transaction.extra_currency_id, 32); // extra currency id
        BitString_storeUint(&ec_bits, ctx->transaction.extra_currency_amount_len, 5); // extra currency amount length
        BitString_storeBuffer(&ec_bits, ctx->transaction.extra_currency_amount_buf, ctx->transaction.extra_currency_amount_len);

        if (!hash_Cell(&ec_bits, NULL, 0, &refs[ref_count++])) {
            return false;
        }
    } else {
        BitString_storeBit(&bits, 0);
    }
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

        refs[ref_count].max_depth = ctx->transaction.state_init.max_depth;
        memmove(refs[ref_count].hash, ctx->transaction.state_init.hash, HASH_LEN);
        ref_count++;

        refs[ref_count].max_depth = ctx->transaction.payload.max_depth;
        memmove(refs[ref_count].hash, ctx->transaction.payload.hash, HASH_LEN);
        ref_count++;

        // Hash cell
        if (!hash_Cell(&bits, refs, ref_count, &internalMessageRef)) {
            return false;
        }
    } else if (ctx->transaction.has_payload) {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 1);  // body in ref

        // Create ref
        refs[ref_count].max_depth = ctx->transaction.payload.max_depth;
        memmove(refs[ref_count].hash, ctx->transaction.payload.hash, HASH_LEN);
        ref_count++;

        // Hash cell
        if (!hash_Cell(&bits, refs, ref_count, &internalMessageRef)) {
            return false;
        }
    } else if (ctx->transaction.has_state_init) {
        BitString_storeBit(&bits, 1);  // no state-init
        BitString_storeBit(&bits, 1);  // state-init ref
        BitString_storeBit(&bits, 0);  // body inline

        // Create ref
        refs[ref_count].max_depth = ctx->transaction.state_init.max_depth;
        memmove(refs[ref_count].hash, ctx->transaction.state_init.hash, HASH_LEN);
        ref_count++;

        // Hash cell
        if (!hash_Cell(&bits, refs, ref_count, &internalMessageRef)) {
            return false;
        }
    } else {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 0);  // body inline

        // Hash cell
        if (!hash_Cell(&bits, refs, ref_count, &internalMessageRef)) {
            return false;
        }
    }

    //
    // Order
    //

    struct CellRef_t orderRef;
    BitString_init(&bits);
    BitString_storeUint(&bits, ctx->transaction.subwallet_id, 32);  // Wallet ID
    BitString_storeUint(&bits, ctx->transaction.timeout, 32);       // Timeout
    BitString_storeUint(&bits, ctx->transaction.seqno, 32);         // Seqno
    if (transaction_include_wallet_op(&ctx->transaction)) {
        BitString_storeUint(&bits, 0, 8);  // Simple order
    }
    BitString_storeUint(&bits, ctx->transaction.send_mode, 8);  // Send Mode
    struct CellRef_t orderRefs[1] = {internalMessageRef};
    if (!hash_Cell(&bits, orderRefs, 1, &orderRef)) {
        return false;
    }

    // Result
    memmove(ctx->m_hash, orderRef.hash, HASH_LEN);

    return true;
}