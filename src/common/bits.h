#pragma once

#include <stdint.h>  // uint*_t
#include <stdbool.h>
#include <stddef.h>

#include "types.h"

void BitString_init(BitString_t* self);
void BitString_storeBit(BitString_t* self, int8_t v);
void BitString_storeUint(BitString_t* self, uint64_t v, uint8_t bits);
void BitString_storeCoins(BitString_t* self, uint64_t v);
void BitString_storeCoinsBuf(BitString_t* self, uint8_t* v, uint8_t len);
void BitString_storeBuffer(BitString_t* self, const uint8_t* v, uint8_t length);
void BitString_storeAddress(BitString_t* self, uint8_t chain, uint8_t* hash);
void BitString_storeAddressNull(BitString_t* self);
void BitString_finalize(BitString_t* self);
