#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

/**
 * Calculate CRC-CCITT (not the normal CRC16!) for the given data.
 *
 * @param[in]  ptr
 *   Pointer to input byte buffer.
 * @param[in]  count
 *   Length of the input byte buffer.
 *
 * @return CRC-CCITT for the given data.
 *
 */
uint16_t crc16(uint8_t *ptr, size_t count);
