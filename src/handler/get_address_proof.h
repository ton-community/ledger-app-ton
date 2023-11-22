#pragma once

#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "../common/buffer.h"

/**
 * Handler for GET_ADDRESS_PROOF command. If successfully parse BIP32 path,
 * derive public key, sign a TON Connect proof message and send APDU response.
 *
 * @see G_context.bip32_path, G_context.proof_info.raw_public_key
 *
 * @param[in]     flags
 *   Address display flags
 * @param[in,out] cdata
 *   Command data with BIP32 path.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_address_proof(uint8_t flags, buffer_t *cdata);
