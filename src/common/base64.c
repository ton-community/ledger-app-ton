#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

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
