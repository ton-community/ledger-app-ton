#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t

#include "encoding.h"

bool check_ascii(const uint8_t *memo, size_t memo_len) {
    for (size_t i = 0; i < memo_len; i++) {
        if (memo[i] >= 0x7F) {
            return false;
        }
        if (memo[i] < 0x20) {
            return false;
        }
    }

    return true;
}
