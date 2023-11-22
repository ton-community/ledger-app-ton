#include <stdint.h>  // uint*_t
#include <stdbool.h>

#include "cx.h"

#include "cell.h"

#include "bits.h"
#include "../constants.h"

#define SAFE(RES)         \
    if ((RES) != CX_OK) { \
        return false;     \
    }

bool hash_Cell(BitString_t *bits, CellRef_t *refs, uint8_t refs_count, CellRef_t *out) {
    cx_sha256_t state;
    SAFE(cx_sha256_init_no_throw(&state));

    // Data and descriptors
    uint16_t len = bits->data_cursor;
    uint8_t d1 = refs_count;                     // refs descriptor
    uint8_t d2 = (len >> 3) + ((len + 7) >> 3);  // bits descriptor
    uint8_t d[2] = {d1, d2};
    BitString_finalize(bits);

    // Hash data and descriptors
    SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, d, 2, NULL, 0));
    SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, bits->data, bits->data_cursor / 8, NULL, 0));

    // Hash ref depths
    for (int i = 0; i < refs_count; i++) {
        struct CellRef_t md = refs[i];
        uint8_t mdd[2] = {md.max_depth / 256, md.max_depth % 256};
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, mdd, 2, NULL, 0));
    }

    // Hash ref depths
    for (int i = 0; i < refs_count; i++) {
        struct CellRef_t md = refs[i];
        SAFE(cx_hash_no_throw((cx_hash_t *) &state, 0, md.hash, HASH_LEN, NULL, 0));
    }

    // Finalize
    SAFE(cx_hash_no_throw((cx_hash_t *) &state, CX_LAST, NULL, 0, out->hash, HASH_LEN));

    // Depth
    out->max_depth = 0;
    if (refs_count > 0) {
        for (int i = 0; i < refs_count; i++) {
            struct CellRef_t md = refs[i];
            if (md.max_depth > out->max_depth) {
                out->max_depth = md.max_depth;
            }
        }
        out->max_depth = out->max_depth + 1;
    }

    return true;
}