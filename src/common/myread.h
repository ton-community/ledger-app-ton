#pragma once

#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t

/**
 * Read 6 bytes as Big Endian from byte buffer.
 *
 * @param[in] ptr
 *   Pointer to byte buffer.
 * @param[in] offset
 *   Offset in the byte buffer.
 *
 * @return 8 bytes value read from buffer.
 *
 */
uint64_t read_u48_be(const uint8_t *ptr, size_t offset);

/**
 * Read 6 bytes as Little Endian from byte buffer.
 *
 * @param[in] ptr
 *   Pointer to byte buffer.
 * @param[in] offset
 *   Offset in the byte buffer.
 *
 * @return 8 bytes value read from buffer.
 *
 */
uint64_t read_u48_le(const uint8_t *ptr, size_t offset);
