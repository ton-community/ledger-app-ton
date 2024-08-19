#ifdef HAVE_SWAP

#include <string.h>

#include "os.h"
#include "crypto_helpers.h"
#include "swap.h"
#include "../types.h"
#include "common/mybuffer.h"
#include "get_public_key.h"
#include "base64.h"
#include "address.h"
#include "constants.h"
#include "crc16.h"

#define BASE_CHAIN   0x00
#define MASTER_CHAIN 0xFF

/* Set params.result to 0 on error, 1 otherwise */
void swap_handle_check_address(check_address_parameters_t *params) {
    PRINTF("Inside Ton swap_handle_check_address\n");
    params->result = 0;

    if (params->address_parameters == NULL) {
        PRINTF("derivation path expected\n");
        return;
    }
    PRINTF("address_parameters %.*H\n",
           params->address_parameters_length,
           params->address_parameters);

    if (params->address_to_check == NULL) {
        PRINTF("Address to check expected\n");
        return;
    }
    PRINTF("Address to check %s\n", params->address_to_check);
    if (strlen(params->address_to_check) != 48) {
        PRINTF("Address to check expected length 48, not %d\n", strlen(params->address_to_check));
        return;
    }

    buffer_t cdata;
    cdata.ptr = params->address_parameters;
    cdata.size = params->address_parameters_length;
    cdata.offset = 0;

    uint8_t bip32_path_len;
    uint32_t bip32_path[MAX_BIP32_PATH];
    pubkey_ctx_t pk_info;
    pk_info.subwallet_id = 698983191;
    pk_info.is_v3r2 = false;
    if (get_public_key_helper(0, &cdata, &bip32_path_len, bip32_path, &pk_info) != 0) {
        PRINTF("Failed to read public key\n");
        return;
    }
    PRINTF("Derived on path %.*H\n", 4 * bip32_path_len, bip32_path);
    PRINTF("Public key %.*H\n", PUBKEY_LEN, pk_info.raw_public_key);

    uint8_t hash[HASH_LEN] = {0};
    if (!pubkey_to_hash(pk_info.raw_public_key,
                        pk_info.subwallet_id,
                        pk_info.is_v3r2,
                        hash,
                        sizeof(hash))) {
        return;
    }
    PRINTF("hash %.*H\n", HASH_LEN, hash);

    char address_base64url[48];
    if (base64_to_base64url(params->address_to_check, 48, address_base64url, 48) < 0) {
        PRINTF("Failed to convert to base64url\n");
        return;
    }
    uint8_t address_decoded[36];
    int ret = base64url_decode(address_base64url, 48, address_decoded, 36);
    if (ret != 36) {
        PRINTF("%d\n", ret);
        PRINTF("Failed to decode\n");
        return;
    }
    PRINTF("address_decoded = %.*H\n", 36, address_decoded);

    uint8_t flag = address_decoded[0];
    uint8_t workchain_id = address_decoded[1];
    uint8_t *account_id = &address_decoded[2];
    uint16_t address_verification = U2BE(address_decoded, 34);

    if (flag & NON_BOUNCEABLE) {
        PRINTF("Setting mode NON_BOUNCEABLE\n");
        flag &= ~NON_BOUNCEABLE;
    } else if (flag & BOUNCEABLE) {
        PRINTF("Setting mode BOUNCEABLE\n");
        flag &= ~BOUNCEABLE;
    } else {
        PRINTF("Invalid flag value for bounceability %d\n", flag);
        return;
    }

    if (flag & TESTNET_ONLY) {
        PRINTF("Testnet only address refused, flag: %d\n", flag);
        return;
    }

    if (flag != 0) {
        PRINTF("Unknown flag refused: %d\n", flag);
        return;
    }

    if (workchain_id != BASE_CHAIN && workchain_id != MASTER_CHAIN) {
        PRINTF("Unknown workchain_id refused: %d\n", workchain_id);
        return;
    }

    if (memcmp(account_id, hash, HASH_LEN) != 0) {
        PRINTF("Different account id value: received %.*H, derived %.*H\n",
               HASH_LEN,
               account_id,
               HASH_LEN,
               hash);
        PRINTF("Path was %.*H\n", params->address_parameters_length, params->address_parameters);
        return;
    }

    uint16_t calculated_address_verification = crc16(address_decoded, 34);
    if (calculated_address_verification != address_verification) {
        PRINTF("Wrong address verification: calculated %d, received %d\n",
               calculated_address_verification,
               address_verification);
        return;
    }

    PRINTF("Addresses match\n");

    params->result = 1;
}

#endif  // HAVE_SWAP
