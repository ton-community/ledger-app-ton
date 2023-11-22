#pragma once

#include <stdbool.h>

#include "../types.h"

/**
 * Computes transaction hash for signing
 *
 * @param[out]     tx
 *   Pointer to transaction structure.
 *
 * @return true if success, false otherwise.
 *
 */
bool hash_tx(transaction_ctx_t *ctx);
