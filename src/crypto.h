#pragma once

#include <stdint.h>  // uint*_t

#include "constants.h"

/**
 * Initialize public key given bip32 path.
 *
 * @param[in]  bip32_path
 *   Pointer to buffer with BIP32 path.
 * @param[in]  bip32_path_len
 *   Number of path in BIP32 path.
 * @param[out] raw_public_key
 *   Pointer to raw public key.
 *
 * @return 0 if success, -1 otherwise.
 *
 */
int crypto_derive_public_key(const uint32_t *bip32_path,
                             uint8_t bip32_path_len,
                             uint8_t raw_public_key[static PUBKEY_LEN]);

/**
 * Sign tx hash in global context.
 *
 * @see G_context.bip32_path, G_context.tx_info.m_hash,
 * G_context.tx_info.signature.
 *
 * @return 0 if success, -1 otherwise.
 *
 */
int crypto_sign_tx(void);

/**
 * Sign proof hash in global context.
 *
 * @see G_context.bip32_path, G_context.proof_info.hash,
 * G_context.proof_info.signature.
 *
 * @return 0 if success, -1 otherwise.
 *
 */
int crypto_sign_proof(void);

/**
 * Sign custom data in global context.
 *
 * @see G_context.bip32_path, G_context.sign_data_info.cell_hash,
 * G_context.sign_data_info.schema_crc, G_context.sign_data_info.timestamp,
 * G_context.sign_data_info.signature.
 *
 * @return 0 if success, -1 otherwise.
 *
 */
int crypto_sign_sign_data(void);
