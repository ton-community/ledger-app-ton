#pragma once

#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "common/mybuffer.h"

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
