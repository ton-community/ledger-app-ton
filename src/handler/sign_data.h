#pragma once

#include <stdbool.h>  // bool

#include "common/mybuffer.h"

/**
 * Handler for SIGN_DATA command. If successfully parse BIP32 path
 * and custom data, sign custom data and send APDU response.
 *
 * @param[in,out] cdata
 *   Command data with BIP32 path and raw transaction serialized.
 * @param[in]     first
 *   Whether this is the first chunk or not
 * @param[in]     more
 *   Whether more APDU chunks are to be received or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_sign_data(buffer_t *cdata, bool first, bool more);
