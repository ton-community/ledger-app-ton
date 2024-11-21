#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "format_address.h"

#include "../transaction/types.h"
#include "crc16.h"
#include "../constants.h"

bool address_to_friendly(const uint8_t chain,
                         const uint8_t hash[static HASH_LEN],
                         const bool bounceable,
                         const bool testOnly,
                         uint8_t *out,
                         size_t out_len) {
    if (out_len < ADDRESS_LEN) {
        return false;
    }

    // Address Tag
    if (bounceable) {
        out[0] = BOUNCEABLE;
    } else {
        out[0] = NON_BOUNCEABLE;
    }
    if (testOnly) {
        out[0] = out[0] | TESTNET_ONLY;
    }

    // Workchain
    out[1] = chain;

    // Hash
    memmove(out + 2, hash, 32);

    // crc16
    uint16_t crc = crc16(out, 34);
    out[34] = (crc >> 8) & 0xff;
    out[35] = crc & 0xff;

    return true;
}
