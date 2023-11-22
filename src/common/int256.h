#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#define INT256_LENGTH 32

/**
 * Convert big endian number into its decimal representation.
 *
 * @param[in] value
 *   Pointer to number buffer.
 * @param[in] value_len
 *   Length of number buffer.
 * @param[out] out
 *   Output string.
 * @param[in] out_len
 *   Output string buffer size.
 *
 * @return true if success, false otherwise.
 *
 */
bool uint256_to_decimal(const uint8_t *value, size_t value_len, char *out, size_t out_len);

static __attribute__((no_instrument_function)) inline int allzeroes(void *buf, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    for (size_t i = 0; i < n; ++i) {
        if (p[i]) {
            return 0;
        }
    }
    return 1;
}
