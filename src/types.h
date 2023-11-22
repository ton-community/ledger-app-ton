#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <stdbool.h>

#include "constants.h"
#include "transaction/types.h"
#include "common/bip32.h"

/**
 * Enumeration for the status of IO.
 */
typedef enum {
    READY,     /// ready for new event
    RECEIVED,  /// data received
    WAITING    /// waiting
} io_state_e;

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_VERSION = 0x03,        /// version of the application
    GET_APP_NAME = 0x04,       /// name of the application
    GET_PUBLIC_KEY = 0x05,     /// public key of corresponding BIP32 path
    SIGN_TX = 0x06,            /// sign transaction with BIP32 path
    GET_ADDRESS_PROOF = 0x08,  /// get an address proof in TON Connect format
    SIGN_DATA = 0x09,          /// sign data in TON Connect format
} command_e;

/**
 * Structure with fields of APDU command.
 */
typedef struct {
    uint8_t cla;    /// Instruction class
    command_e ins;  /// Instruction code
    uint8_t p1;     /// Instruction parameter 1
    uint8_t p2;     /// Instruction parameter 2
    uint8_t lc;     /// Length of command data
    uint8_t *data;  /// Command data
} command_t;

/**
 * Enumeration with parsing state.
 */
typedef enum {
    STATE_NONE,     /// No state
    STATE_PARSED,   /// Transaction data parsed
    STATE_APPROVED  /// Transaction data approved
} state_e;

/**
 * Enumeration with user request type.
 */
typedef enum {
    CONFIRM_ADDRESS,      /// confirm address derived from public key
    CONFIRM_TRANSACTION,  /// confirm transaction information
    GET_PROOF,            /// confirm address proof information
    CONFIRM_SIGN_DATA,    /// confirm data for signing in TON Connect format
} request_type_e;

/**
 * Structure for public key context information.
 */
typedef struct {
    uint8_t raw_public_key[PUBKEY_LEN];
} pubkey_ctx_t;

/**
 * Structure for transaction information context.
 */
typedef struct {
    uint8_t raw_tx[MAX_TRANSACTION_LEN];  /// raw transaction serialized
    size_t raw_tx_len;                    /// length of raw transaction
    transaction_t transaction;            /// structured transaction
    uint8_t m_hash[HASH_LEN];             /// message hash digest
    uint8_t signature[SIG_LEN];           /// transaction signature
} transaction_ctx_t;

/**
 * Structure for address proof information context.
 */
typedef struct {
    uint8_t raw_public_key[PUBKEY_LEN];
    int32_t workchain;
    uint8_t domain[MAX_DOMAIN_LEN];
    uint8_t domain_len;
    uint8_t hash[HASH_LEN];
    uint8_t signature[SIG_LEN];
    uint8_t address_hash[HASH_LEN];
} proof_ctx_t;

typedef struct {
    uint32_t schema_crc;
    uint64_t timestamp;
    size_t raw_data_len;
    uint8_t raw_data[MAX_DATA_LEN];
    uint8_t cell_hash[HASH_LEN];
    uint8_t signature[SIG_LEN];
    HintHolder_t hints;
} sign_data_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    state_e state;  /// state of the context
    union {
        pubkey_ctx_t pk_info;       /// public key context
        transaction_ctx_t tx_info;  /// transaction context
        proof_ctx_t proof_info;
        sign_data_ctx_t sign_data_info;
    };
    request_type_e req_type;              /// user request
    uint32_t bip32_path[MAX_BIP32_PATH];  /// BIP32 path
    uint8_t bip32_path_len;               /// length of BIP32 path
} global_ctx_t;

typedef struct {
    bool initialized;
    bool expert_mode;
} internalStorage_t;
