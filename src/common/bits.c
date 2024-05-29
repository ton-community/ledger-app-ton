#include <stdint.h>  // uint*_t
#include <string.h>  // explicit_bzero

#include "bits.h"
#include "../constants.h"
#include "cell.h"

void BitString_init(BitString_t* self) {
    self->data_cursor = 0;
    explicit_bzero(self->data, sizeof(self->data));
}

void BitString_storeBit(BitString_t* self, int8_t v) {
    if (v > 0) {
        // this.#buffer[(n / 8) | 0] |= 1 << (7 - (n % 8));
        self->data[(self->data_cursor / 8) | 0] |= (1 << (7 - (self->data_cursor % 8)));
    } else {
        // this.#buffer[(n / 8) | 0] &= ~(1 << (7 - (n % 8)));
        self->data[(self->data_cursor / 8) | 0] &= ~(1 << (7 - (self->data_cursor % 8)));
    }
    self->data_cursor++;
}

void BitString_storeUint(BitString_t* self, uint64_t v, uint8_t bits) {
    for (int i = 0; i < bits; i++) {
        int8_t b = (v >> (bits - i - 1)) & 0x01;
        BitString_storeBit(self, b);
    }
}

void BitString_storeCoins(BitString_t* self, uint64_t v) {
    // Measure length
    uint8_t len = 0;
    uint64_t r = v;
    for (int i = 0; i < 8; i++) {
        if (r > 0) {
            len++;
            r = r >> 8;
        } else {
            break;
        }
    }

    // Write length
    BitString_storeUint(self, len, 4);

    // Write remaining
    for (int i = 0; i < len; i++) {
        BitString_storeUint(self, v >> ((len - i - 1) * 8), 8);
    }
}

void BitString_storeCoinsBuf(BitString_t* self, uint8_t* v, uint8_t len) {
    // Write length
    BitString_storeUint(self, len, 4);

    // Write remaining
    BitString_storeBuffer(self, v, len);
}

void BitString_storeBuffer(BitString_t* self, const uint8_t* v, uint8_t length) {
    for (int i = 0; i < length; i++) {
        BitString_storeUint(self, v[i], 8);
    }
}

void BitString_storeAddress(BitString_t* self, uint8_t chain, uint8_t* hash) {
    BitString_storeUint(self, 2, 2);
    BitString_storeUint(self, 0, 1);
    BitString_storeUint(self, chain, CHAIN_LEN * 8);
    BitString_storeBuffer(self, hash, HASH_LEN);
}

void BitString_storeAddressNull(BitString_t* self) {
    BitString_storeUint(self, 0, 2);
}

void BitString_finalize(BitString_t* self) {
    uint8_t padBytes = self->data_cursor % 8;
    if (padBytes > 0) {
        padBytes = 8 - padBytes;
        padBytes = padBytes - 1;
        BitString_storeBit(self, 1);
        while (padBytes > 0) {
            padBytes = padBytes - 1;
            BitString_storeBit(self, 0);
        }
    }
}
