#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include <string.h>
#include "base64.h"

static const char base64_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const size_t base64_mod_table[] = {0, 2, 1};

int base64_encode(const uint8_t *data, size_t data_length, char *out, size_t out_len) {
    int out_length = 4 * ((data_length + 2) / 3);
    if (((size_t) out_length) + 1 > out_len) {
        return -1;
    }
    for (size_t i = 0, j = 0; i < data_length;) {
        uint32_t octet_a = i < data_length ? data[i++] : 0;
        uint32_t octet_b = i < data_length ? data[i++] : 0;
        uint32_t octet_c = i < data_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        out[j++] = base64_alphabet[(triple >> 3 * 6) & 0x3F];
        out[j++] = base64_alphabet[(triple >> 2 * 6) & 0x3F];
        out[j++] = base64_alphabet[(triple >> 1 * 6) & 0x3F];
        out[j++] = base64_alphabet[(triple >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < base64_mod_table[data_length % 3]; i++) {
        out[out_length - 1 - i] = '=';
    }

    out[out_length] = '\0';
    return out_length;
}

const uint8_t pr2six[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 63,  //
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,  //
    64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,  //
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 63,  //
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,  //
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64   //
};

// Taken from Exchange application implementation
int base64url_decode(const char *src, size_t src_len, uint8_t *dest, size_t dest_len) {
    const uint8_t *original_dest;

    original_dest = dest;

    if (src_len % 4 == 1) {
        return -1;
    } else if (src_len == 0) {
        return 0;
    }

    while (src_len > 4) {
        if (dest_len < 3) {
            return -1;
        }
        *(dest++) = (pr2six[(uint8_t) src[0]] << 2 | pr2six[(uint8_t) src[1]] >> 4);
        *(dest++) = (pr2six[(uint8_t) src[1]] << 4 | pr2six[(uint8_t) src[2]] >> 2);
        *(dest++) = (pr2six[(uint8_t) src[2]] << 6 | pr2six[(uint8_t) src[3]] >> 0);
        src += 4;
        src_len -= 4;
        dest_len -= 3;
    }

    if (src_len > 1) {
        if (dest_len < 1) {
            return -1;
        }
        *(dest++) = (pr2six[(uint8_t) src[0]] << 2 | pr2six[(uint8_t) src[1]] >> 4);
        dest_len--;
    }

    if (src_len > 2) {
        if (dest_len < 1) {
            return -1;
        }
        *(dest++) = (pr2six[(uint8_t) src[1]] << 4 | pr2six[(uint8_t) src[2]] >> 2);
        dest_len--;
    }

    if (src_len > 3) {
        if (dest_len < 1) {
            return -1;
        }
        *(dest++) = (pr2six[(uint8_t) src[2]] << 6 | pr2six[(uint8_t) src[3]] >> 0);
    }

    return dest - original_dest;
}

int base64_to_base64url(const char *src, size_t src_len, char *dest, size_t dest_len) {
    if (dest_len < src_len) {
        return -1;
    }
    explicit_bzero(dest, dest_len);

    size_t i;
    for (i = 0; i < src_len; i++) {
        switch (src[i]) {
            case '+':
                dest[i] = '-';
                break;
            case '/':
                dest[i] = '_';
                break;
            case '=':
                // Padding character only found at the end, we simply remove it
                dest[i] = '\0';
                return i;
            default:
                dest[i] = src[i];  // Copy other characters directly
                break;
        }
    }

    return src_len;
}
