#include <stdbool.h>

#include "../globals.h"

#include "bip32_check.h"

bool check_bip32_path(uint8_t bip32_path_len, uint32_t bip32_path[MAX_BIP32_PATH]) {
    if (bip32_path_len <= 2) return false;

    return bip32_path[0] == 0x8000002c && bip32_path[1] == 0x8000025f;
}

bool check_global_bip32_path() {
    return check_bip32_path(G_context.bip32_path_len, G_context.bip32_path);
}
