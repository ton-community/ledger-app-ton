#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#include "constants.h"

/**
 * Convert public key to address hash. Uses Wallet V4 contract.
 *
 * @param[in]  public_key
 *   Pointer to byte buffer with public key.
 *   The public key is represented as 32 bytes.
 * @param[out] out
 *   Pointer to output byte buffer for address.
 * @param[in]  out_len
 *   Length of output byte buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool pubkey_to_hash(const uint8_t public_key[static PUBKEY_LEN], uint8_t *out, size_t out_len);

/**
 * Convert public key to address. Uses Wallet V4 contract.
 *
 * @param[in]  public_key
 *   Pointer to byte buffer with public key.
 *   The public key is represented as 32 bytes.
 * @param[in]  chain
 *   0x00 or 0xff for workchains
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
bool address_from_pubkey(const uint8_t public_key[static PUBKEY_LEN],
                         const uint8_t chain,
                         const bool bounceable,
                         const bool testOnly,
                         uint8_t *out,
                         size_t out_len);
