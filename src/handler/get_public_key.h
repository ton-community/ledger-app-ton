#pragma once

#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "../types.h"
#include "common/mybuffer.h"

/**
 * Helper for GET_PUBLIC_KEY handler. If successfully parse BIP32 path and derive the public key.
 *
 * @param[in]     flags
 *   Address display flags
 * @param[in,out] cdata
 *   Command data with BIP32 path.
 * @param[out]     bip32_path_len
 *   Requested bip32_path to derive the public key on length
 * @param[out]     bip32_path
 *   Requested bip32_path to derive the public key on
 * @param[out]     pk_info
 *   Additional info to derive the public key with
 *
 * @return zero if success, error code otherwise.
 *
 */
int get_public_key_helper(uint8_t flags,
                          buffer_t *cdata,
                          uint8_t *bip32_path_len,
                          uint32_t bip32_path[MAX_BIP32_PATH],
                          pubkey_ctx_t *pk_info);

/**
 * Handler for GET_PUBLIC_KEY command. If successfully parse BIP32 path,
 * derive public key and send APDU response.
 *
 * @see G_context.bip32_path, G_context.pk_info.raw_public_key
 *
 * @param[in]     flags
 *   Address display flags
 * @param[in,out] cdata
 *   Command data with BIP32 path.
 * @param[in]     display
 *   Whether to display address on screen or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_public_key(uint8_t flags, buffer_t *cdata, bool display);
