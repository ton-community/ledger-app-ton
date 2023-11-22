#pragma once

#include <stdbool.h>  // bool

/**
 * Check the bip32 path stored in G_context.
 *
 * @return true if bip32 path is valid, false otherwise
 *
 */
bool check_global_bip32_path();
