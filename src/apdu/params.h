#pragma once

/**
 * P1 indicating no information.
 */
#define P1_NONE 0x00

/**
 * P1 indicating a request to confirm an address or an address proof.
 */
#define P1_CONFIRM 0x01

/**
 * P1 indicating a request to get the public key without confirmation.
 */
#define P1_NON_CONFIRM 0x00

/**
 * P2 indicating no information.
 */
#define P2_NONE 0x00

/**
 * P2 indicating first APDU in a large request.
 */
#define P2_FIRST 0x01

/**
 * P2 indicating that this is not the last APDU in a large request.
 */
#define P2_MORE 0x02

/**
 * P2 bit indicating that address should be displayed as testnet only.
 */
#define P2_ADDR_FLAG_TESTNET 0x01

/**
 * P2 bit indicating that masterchain address must be generated.
 */
#define P2_ADDR_FLAG_MASTERCHAIN 0x02

/**
 * P2 bit indicating that wallet specifiers (subwallet_id and version) are present.
 */
#define P2_ADDR_FLAG_WALLET_SPECIFIERS 0x04

/**
 * P2 containing all address display bits.
 */
#define P2_ADDR_FLAGS_MAX \
    (P2_ADDR_FLAG_TESTNET | P2_ADDR_FLAG_MASTERCHAIN | P2_ADDR_FLAG_WALLET_SPECIFIERS)
