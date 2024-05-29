#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Format byte buffer to uppercase hexadecimal string.
 *
 * @param[in]  in
 *   Pointer to input byte buffer.
 * @param[in]  in_len
 *   Length of input byte buffer.
 * @param[out] out
 *   Pointer to output string.
 * @param[in]  out_len
 *   Length of output string.
 *
 * @return number of bytes written if success, -1 otherwise.
 *
 */
int format_hex(const uint8_t *in, size_t in_len, char *out, size_t out_len);

void format_u64(uint64_t in, char *out, size_t out_len);
