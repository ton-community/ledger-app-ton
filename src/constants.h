#pragma once

/**
 * Instruction class of the Boilerplate application.
 */
#define CLA 0xE0

/**
 * Length of APPNAME variable in the Makefile.
 */
#define APPNAME_LEN (sizeof(APPNAME) - 1)

/**
 * Maximum length of MAJOR_VERSION || MINOR_VERSION || PATCH_VERSION.
 */
#define APPVERSION_LEN 3

/**
 * Maximum length of application name.
 */
#define MAX_APPNAME_LEN 64

/**
 * Maximum transaction length (bytes).
 */
#define MAX_TRANSACTION_LEN 510

/**
 * Signature length (bytes).
 */
#define SIG_LEN 64

/**
 * Cell Hash length (bytes).
 */
#define HASH_LEN 32

/**
 * Workchain ID length (bytes).
 */
#define CHAIN_LEN 1

/**
 * Exponent used to convert nanoTON to TON unit (N TON = N * 10^9 nanoTON).
 */
#define EXPONENT_SMALLEST_UNIT 9

/**
 * Max bytes allowed in TON's conventional encoding for big numbers.
 */
#define MAX_VALUE_BYTES_LEN 15

/**
 * Max ticker string length.
 */
#define MAX_TICKER_LEN 16

/**
 * Max domain string length.
 */
#define MAX_DOMAIN_LEN 128

/**
 * Raw public key length.
 */
#define PUBKEY_LEN 32

/**
 * Raw private key length.
 */
#define PRIVKEY_LEN 64

/**
 * Max hints in one transaction.
 */
#define MAX_HINTS 8

/**
 * Maximum signed data length (bytes).
 */
#define MAX_DATA_LEN 510

/**
 * Length of address in user-friendly form before base64 encoding (bytes).
 */
#define ADDRESS_LEN 36
