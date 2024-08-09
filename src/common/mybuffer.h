#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#include "buffer.h"
#include "types.h"

/**
 * Read 1 byte boolean into bool.
 *
 * @param[in,out]  buffer
 *   Pointer to input buffer struct.
 * @param[out]     value
 *   Pointer to 8-bit unsigned integer read from buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_bool(buffer_t *buffer, bool *value);

/**
 * Read 6 bytes from buffer into uint64_t.
 *
 * @param[in,out]  buffer
 *   Pointer to input buffer struct.
 * @param[out]     value
 *   Pointer to 64-bit unsigned integer read from buffer.
 * @param[in]      endianness
 *   Either BE (Big Endian) or LE (Little Endian).
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_u48(buffer_t *buffer, uint64_t *value, endianness_t endianness);

/**
 * Set out to point to the remaining contents of the buffer.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 * @param[out] out
 *   Pointer to output byte buffer.
 * @param[in]  out_len
 *   Length of output byte buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_ref(buffer_t *buffer, uint8_t **out, size_t out_len);

/**
 * Move some bytes from buffer.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 * @param[out] out
 *   Pointer to output byte buffer.
 * @param[in]  out_len
 *   Length of output byte buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_buffer(buffer_t *buffer, uint8_t *out, size_t out_len);

/**
 * Read variable length integer from buffer.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 * @param[out] out_size
 *   Pointer to variable length integer size.
 * @param[out] out
 *   Pointer to output byte buffer.
 * @param[in]  out_len
 *   Length of output byte buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_varuint(buffer_t *buffer, uint8_t *out_size, uint8_t *out, size_t out_len);

/**
 * Tell how many bytes are left in buffer.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 *
 * @return amount of bytes left in buffer
 *
 */
size_t buffer_remaining(const buffer_t *buffer);

/**
 * Read serialized address (33 bytes) from buffer.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 * @param[out] out
 *   Pointer to output address.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_address(buffer_t *buf, address_t *out);

/**
 * Read serialized cell reference (34 bytes) from buffer.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 * @param[out] out
 *   Pointer to output cell reference.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_read_cell_ref(buffer_t *buf, CellRef_t *out);
