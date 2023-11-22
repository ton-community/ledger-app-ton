#pragma once

#include <stdbool.h>

#include "types.h"
#include "../common/buffer.h"
#include "../common/types.h"

bool buffer_read_address(buffer_t *buf, address_t *out);

bool buffer_read_cell_ref(buffer_t *buf, CellRef_t *out);

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx
 *   Pointer to transaction structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e transaction_deserialize(buffer_t *buf, transaction_t *tx);
