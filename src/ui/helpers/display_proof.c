#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display_proof.h"

#include "../../transaction/types.h"
#include "../../common/format_address.h"
#include "../../common/encoding.h"
#include "../../globals.h"
#include "io.h"
#include "../../sw.h"
#include "../../common/base64.h"
#include "../../apdu/params.h"

bool display_proof(uint8_t flags,
                   char *g_address,
                   size_t g_address_len,
                   char *g_domain,
                   size_t g_domain_len,
                   char *g_address_title,
                   size_t g_address_title_len) {
    memset(g_address, 0, g_address_len);
    uint8_t address[ADDRESS_LEN] = {0};
    bool testnet = false;
    if (flags & P2_ADDR_FLAG_TESTNET) {
        testnet = true;
    }
    if (!address_to_friendly(G_context.proof_info.workchain == -1 ? 0xff : 0,
                             G_context.proof_info.address_hash,
                             false,
                             testnet,
                             address,
                             sizeof(address))) {
        io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
        return false;
    }
    base64_encode(address, sizeof(address), g_address, g_address_len);

    if (check_ascii(G_context.proof_info.domain, G_context.proof_info.domain_len)) {
        memmove(g_domain, G_context.proof_info.domain, G_context.proof_info.domain_len);
        g_domain[G_context.proof_info.domain_len] = '\0';
    } else {
        snprintf(g_domain, g_domain_len, "<cannot display>");
    }

    if (g_address_title != NULL) {
        snprintf(g_address_title, g_address_title_len, "Address");
    }

    return true;
}
