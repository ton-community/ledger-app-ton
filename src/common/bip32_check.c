#include <stdbool.h>

#include "../globals.h"

#include "bip32_check.h"

bool check_global_bip32_path() {
    if (G_context.bip32_path_len <= 2) return false;

    return G_context.bip32_path[0] == 0x8000002c && G_context.bip32_path[1] == 0x8000025f;
}
