#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display_address.h"

#include "../../transaction/types.h"
#include "../../address.h"
#include "../../globals.h"
#include "io.h"
#include "../../sw.h"
#include "../../common/base64.h"
#include "../../apdu/params.h"

bool display_address(uint8_t flags,
                     char *g_address,
                     size_t g_address_len,
                     char *g_address_title,
                     size_t g_address_title_len) {
    memset(g_address, 0, g_address_len);
    uint8_t address[ADDRESS_LEN] = {0};
    bool testnet = false;
    uint8_t chain = 0;
    if (flags & P2_ADDR_FLAG_TESTNET) {
        testnet = true;
    }
    if (flags & P2_ADDR_FLAG_MASTERCHAIN) {
        chain = 0xff;
    }
    if (!address_from_pubkey(G_context.pk_info.raw_public_key,
                             chain,
                             false,
                             testnet,
                             G_context.pk_info.subwallet_id,
                             G_context.pk_info.is_v3r2,
                             address,
                             sizeof(address))) {
        io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
        return false;
    }
    base64_encode(address, sizeof(address), g_address, g_address_len);
    if (g_address_title != NULL) {
        snprintf(g_address_title, g_address_title_len, "Address");
    }
    return true;
}
