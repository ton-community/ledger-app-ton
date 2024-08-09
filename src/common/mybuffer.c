/*****************************************************************************
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "buffer.h"
#include "read.h"
#include "myread.h"
#include "bip32.h"
#include "types.h"

#include "mybuffer.h"

bool buffer_read_bool(buffer_t *buffer, bool *value) {
    if (!buffer_can_read(buffer, 1)) {
        *value = false;

        return false;
    }

    uint8_t v = buffer->ptr[buffer->offset];
    if (v != 0x00 && v != 0x01) {
        *value = false;
        return false;
    }
    *value = (v == 0x01);
    buffer_seek_cur(buffer, 1);
    return true;
}

bool buffer_read_u48(buffer_t *buffer, uint64_t *value, endianness_t endianness) {
    if (!buffer_can_read(buffer, 6)) {
        *value = 0;

        return false;
    }

    *value = ((endianness == BE) ? read_u48_be(buffer->ptr, buffer->offset)
                                 : read_u48_le(buffer->ptr, buffer->offset));

    buffer_seek_cur(buffer, 6);

    return true;
}

size_t buffer_remaining(const buffer_t *buffer) {
    return buffer->size - buffer->offset;
}

bool buffer_read_ref(buffer_t *buffer, uint8_t **out, size_t out_len) {
    if (!buffer_can_read(buffer, out_len)) {
        return false;
    }

    *out = (uint8_t *) (buffer->ptr + buffer->offset);
    buffer_seek_cur(buffer, out_len);
    return true;
}

bool buffer_read_buffer(buffer_t *buffer, uint8_t *out, size_t out_len) {
    if (!buffer_can_read(buffer, out_len)) {
        return false;
    }
    memmove(out, buffer->ptr + buffer->offset, out_len);
    buffer_seek_cur(buffer, out_len);
    return true;
}

bool buffer_read_varuint(buffer_t *buffer, uint8_t *out_size, uint8_t *out, size_t out_len) {
    uint8_t size;
    if (!buffer_read_u8(buffer, &size)) {
        return false;
    }
    if (size > out_len) {
        return false;
    }
    if (!buffer_read_buffer(buffer, out, size)) {
        return false;
    }
    *out_size = size;
    return true;
}

bool buffer_read_address(buffer_t *buf, address_t *out) {
    if (!buffer_read_u8(buf, &out->chain)) {
        return false;
    }
    if (!buffer_read_buffer(buf, out->hash, HASH_LEN)) {
        return false;
    }
    return true;
}

bool buffer_read_cell_ref(buffer_t *buf, CellRef_t *out) {
    if (!buffer_read_u16(buf, &out->max_depth, BE)) {
        return false;
    }
    if (!buffer_read_buffer(buf, out->hash, HASH_LEN)) {
        return false;
    }
    return true;
}
