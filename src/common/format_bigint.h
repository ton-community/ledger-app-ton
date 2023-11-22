#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // int*_t, uint*_t
#include <stdbool.h>  // bool

/**
 * Convert big endian number into its decimal representation, prepending a ticker and adding a
 * decimal point.
 *
 * @param[in] amount
 *   Pointer to number buffer.
 * @param[in] amount_size
 *   Length of number buffer.
 * @param[in] decimals
 *   Number of digits after the decimal point.
 * @param[in] ticker
 *   Ticker string.
 * @param[out] out_buffer
 *   Output string.
 * @param[in] out_buffer_size
 *   Output string buffer size.
 *
 * @return true if success, false otherwise.
 *
 */
bool amountToString(const uint8_t *amount,
                    uint8_t amount_size,
                    uint8_t decimals,
                    const char *ticker,
                    char *out_buffer,
                    size_t out_buffer_size);
