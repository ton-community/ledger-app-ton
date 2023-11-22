#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

/**
 * Convert address to friendly version that could be presented to user.
 *
 * @param[in]  chain
 *   Workchain ID - 0xff or 0x00
 * @param[in]  hash
 *   Pointer to hash part of an address.
 * @param[in]  bounceable
 *   Address have to have bounceable flag set
 * @param[in]  testOnly
 *   Address have to have testnet flag set
 * @param[out] out
 *   Pointer to output byte buffer for address.
 * @param[in]  out_len
 *   Length of output byte buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool address_to_friendly(const uint8_t chain,
                         const uint8_t hash[static 32],
                         const bool bounceable,
                         const bool testOnly,
                         uint8_t *out,
                         size_t out_len);
