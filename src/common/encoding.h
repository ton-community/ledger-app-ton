#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t

/**
 * Check if the data is an ASCII string.
 *
 * @param[in]  memo
 *   Pointer to input byte buffer.
 * @param[in]  memo_len
 *   Length of the input byte buffer.
 *
 * @return true if the data is an ASCII string, false otherwise.
 *
 */
bool check_ascii(const uint8_t *memo, size_t memo_len);
