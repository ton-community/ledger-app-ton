#pragma once

#include <stdbool.h>  // bool

#include "../types.h"  // MAX_BIP32_PATH

/**
 * Check if the given BIP32 path length and path array are valid.
 *
 * @param bip32_path_len The length of the BIP32 path.
 * @param bip32_path The array containing the BIP32 path.
 *
 * @return true if the BIP32 path is valid, false otherwise.
 *
 */
bool check_bip32_path(uint8_t bip32_path_len, uint32_t bip32_path[MAX_BIP32_PATH]);

/**
 * Check the bip32 path stored in G_context.
 *
 * @return true if the BIP32 path is valid, false otherwise.
 *
 */
bool check_global_bip32_path();
