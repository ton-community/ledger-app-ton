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

#define BASE_CHAIN             0x00
#define MASTER_CHAIN           0xFF
#define ADDRESS_BASE64_LENGTH  48
#define ADDRESS_DECODED_LENGTH 36


/**
 * @brief Retrieve address hash according to given derivation path.
 */
static bool swap_get_addr_hash_from_path(
    const check_address_parameters_t * const params,
    uint8_t *hash,
    size_t hash_len
) {
    uint8_t bip32_path_len;
    uint32_t bip32_path[MAX_BIP32_PATH];
    bool ret = false;
    buffer_t cdata = {
        .ptr = params->address_parameters,
        .size = params->address_parameters_length,
        .offset = 0UL,
    };
    pubkey_ctx_t pk_info = {
        .subwallet_id = 698983191,
        .is_v3r2 = false,
    };

    if (params->address_parameters == NULL) {
        PRINTF("derivation path expected\n");
        goto out;
    }

    PRINTF("address_parameters %.*H\n",
           params->address_parameters_length,
           params->address_parameters);

    if (get_public_key_helper(0, &cdata, &bip32_path_len, bip32_path, &pk_info) != 0) {
        PRINTF("Failed to read public key\n");
        goto out;
    }

    PRINTF("Derived on path %.*H\n", 4 * bip32_path_len, bip32_path);
    PRINTF("Public key %.*H\n", PUBKEY_LEN, pk_info.raw_public_key);

    ret = pubkey_to_hash(pk_info.raw_public_key,
                         pk_info.subwallet_id,
                         pk_info.is_v3r2,
                         hash,
                         hash_len);

out:
    return ret;
}

/**
 * @brief Decode the given address in base64 to binary
 *
 * Internally convert base64 address to base64 url and then decode base64 url.APPNAME
 *
 * @param address BASE64 address to decode
 * @param decoded decode buffer
 * @param size decode buffer size
 *
 * @return true if success, false otherwise
 */
static bool swap_decode_address(const char *address, uint8_t *decoded, size_t size) {
    char base64url[ADDRESS_BASE64_LENGTH];
    int result;
    bool ret = false;

    result = base64_to_base64url(address, ADDRESS_BASE64_LENGTH, base64url, ADDRESS_BASE64_LENGTH);
    if (result < 0) {
        PRINTF("Failed to convert to base64url\n");
        goto out;
    }

    result = base64url_decode(base64url, ADDRESS_BASE64_LENGTH, decoded, size);
    if (result != (int)size) {
        PRINTF("%d\n", result);
        goto out;
    }

    ret = true;
out:
    return ret;
}

/**
 * @brief Check address flags validity for Jetton swap
 *
 * Only Bounceable or NonBounceable flag is allowed.
 * Any other value is forbidden (e.g. TestNet)
 *
 * @param flag address flag
 *
 * @return true if valid, false otherwise
 */
static bool swap_check_address_flag(uint8_t flag) {
    if (flag & NON_BOUNCEABLE) {
        PRINTF("Setting mode NON_BOUNCEABLE\n");
        flag &= ~NON_BOUNCEABLE;
    } else if (flag & BOUNCEABLE) {
        PRINTF("Setting mode BOUNCEABLE\n");
        flag &= ~BOUNCEABLE;
    } else {
        PRINTF("Invalid flag value for bounceability %d\n", flag);
        return false;
    }

    if (flag & TESTNET_ONLY) {
        PRINTF("Testnet only address refused, flag: %d\n", flag);
        return false;
    }

    if (flag != 0) {
        PRINTF("Unknown flag refused: %d\n", flag);
        return false;
    }

    return true;
}

/**
 * @brief Check given address consistency
 *
 * Decodes the given address and checks crc, flag and workchain consistency.
 * Compares decoded address hash to this account address.
 *
 * @return true if addresses match, false otherwise
 */
static bool swap_check_address_consistency(
    const check_address_parameters_t * const params,
    const uint8_t *const hash
) {
    uint8_t decoded[ADDRESS_DECODED_LENGTH];
    size_t len;
    uint8_t flag;
    uint8_t workchain_id;
    const uint8_t *account_hash = NULL;
    uint16_t crc, expected_crc;

    bool ret = false;

    if (params->address_to_check == NULL) {
        PRINTF("Address to check expected\n");
        goto out;
    }

    len = strlen(params->address_to_check);
    if (len != ADDRESS_BASE64_LENGTH) {
        PRINTF("Address to check expected length %d, got %u\n", ADDRESS_BASE64_LENGTH, len);
        goto out;
    }

    if (!swap_decode_address(params->address_to_check, decoded, ADDRESS_DECODED_LENGTH)) {
        PRINTF("Failed to decode\n");
        goto out;
    }

    flag = decoded[0];
    workchain_id = decoded[1];
    account_hash = &decoded[2];
    expected_crc = U2BE(decoded, ADDRESS_DECODED_LENGTH - 2);
    crc = crc16(decoded, ADDRESS_DECODED_LENGTH - 2);

    if (crc != expected_crc) {
        PRINTF("Wrong address verification: expected %d, got %d\n", expected_crc, crc);
        goto out;
    }

    if (!swap_check_address_flag(flag)) {
        goto out;
    }

    if (workchain_id != BASE_CHAIN && workchain_id != MASTER_CHAIN) {
        PRINTF("Unknown workchain_id refused: %d\n", workchain_id);
        goto out;
    }

    if (memcmp(account_hash, hash, HASH_LEN) != 0) {
        PRINTF("Different account id value: received %.*H, derived %.*H\n",
               HASH_LEN,
               account_hash,
               HASH_LEN,
               hash);
        goto out;
    }

    ret = true;
out:
    return ret;
}

/* Set params.result to 0 on error, 1 otherwise */
void swap_handle_check_address(check_address_parameters_t *params) {
    uint8_t hash[HASH_LEN] = {0};

    PRINTF("Inside Ton swap_handle_check_address\n");

    // XXX: what if params is null ?!
    params->result = 0;

    if (params->address_to_check == NULL) {
        PRINTF("Address to check expected\n");
        return;
    }
    PRINTF("Address to check %s\n", params->address_to_check);
    if (strlen(params->address_to_check) != ADDRESS_BASE64_LENGTH) {
        PRINTF("Address to check expected length %d, not %d\n",
               ADDRESS_BASE64_LENGTH,
               strlen(params->address_to_check));
        return;
    }

    if (!swap_get_addr_hash_from_path(params, hash, HASH_LEN)) {
        return;
    }

    PRINTF("hash %.*H\n", HASH_LEN, hash);

    if (!swap_check_address_consistency(params, hash)) {
        return;
    }

    PRINTF("Addresses match\n");

    params->result = 1;
}

#endif  // HAVE_SWAP
