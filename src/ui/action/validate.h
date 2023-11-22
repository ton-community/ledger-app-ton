#pragma once

#include <stdbool.h>  // bool

/**
 * Action for public key validation and export.
 *
 * @param[in] choice
 *   User choice (either approved or rejectd).
 *
 */
void ui_action_validate_pubkey(bool choice);

/**
 * Action for transaction information validation.
 *
 * @param[in] choice
 *   User choice (either approved or rejectd).
 *
 */
void ui_action_validate_transaction(bool choice);

/**
 * Action for proof information validation.
 *
 * @param[in] choice
 *   User choice (either approved or rejectd).
 *
 */
void ui_action_validate_proof(bool choice);

/**
 * Action for custom data information validation.
 *
 * @param[in] choice
 *   User choice (either approved or rejectd).
 *
 */
void ui_action_validate_sign_data(bool choice);
