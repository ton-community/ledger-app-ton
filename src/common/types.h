#pragma once

#include <stdint.h>

#include "../constants.h"

typedef struct BitString_t {
    uint8_t data[128];
    uint16_t data_cursor;  // NOTE: In bits
} BitString_t;

typedef struct CellRef_t {
    uint16_t max_depth;
    uint8_t hash[HASH_LEN];
} CellRef_t;

typedef struct {
    uint8_t chain;
    uint8_t hash[HASH_LEN];
} address_t;
