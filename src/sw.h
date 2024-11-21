#pragma once

/**
 * Status word for success.
 */
#define SW_OK 0x9000
/**
 * Status word for denied by user.
 */
#define SW_DENY 0x6985
/**
 * Status word for incorrect P1 or P2.
 */
#define SW_WRONG_P1P2 0x6A86
/**
 * Status word for either wrong Lc or length of APDU command less than 5.
 */
#define SW_WRONG_DATA_LENGTH 0x6A87
/**
 * Status word for unknown command with this INS.
 */
#define SW_INS_NOT_SUPPORTED 0x6D00
/**
 * Status word for instruction class is different than CLA.
 */
#define SW_CLA_NOT_SUPPORTED 0x6E00
/**
 * Status word for wrong response length (buffer too small or too big).
 */
#define SW_WRONG_RESPONSE_LENGTH 0xB000
/**
 * Status word for fail to display address.
 */
#define SW_DISPLAY_ADDRESS_FAIL 0xB002
/**
 * Status word for fail to display amount.
 */
#define SW_DISPLAY_AMOUNT_FAIL 0xB003
/**
 * Status word for wrong transaction length.
 */
#define SW_WRONG_TX_LENGTH 0xB004
/**
 * Status word for failure of transaction parsing.
 */
#define SW_TX_PARSING_FAIL 0xB010
/**
 * Status word for wrong custom data length.
 */
#define SW_WRONG_SIGN_DATA_LENGTH 0xB005
/**
 * Status word for failure of custom data parsing.
 */
#define SW_SIGN_DATA_PARSING_FAIL 0xB011
/**
 * Status word for bad state.
 */
#define SW_BAD_STATE 0xB007
/**
 * Status word for signature fail.
 */
#define SW_SIGNATURE_FAIL 0xB008
/**
 * Status word for a failed swap operation
 */
#define SW_SWAP_FAILURE 0xB009
/**
 * Status word for a request that is too long.
 */
#define SW_REQUEST_TOO_LONG 0xB00B
/**
 * Status word for bad bip32 path.
 */
#define SW_BAD_BIP32_PATH 0xB0BD
/**
 * Status word for a blind signing request when blind signing is disabled.
 */
#define SW_BLIND_SIGNING_DISABLED 0xBD00
