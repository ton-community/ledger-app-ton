#pragma once

#include <stdbool.h>  // bool
#include <stdint.h>

#define G_ADDRESS_LEN       49
#define G_OPERATION_LEN     64
#define G_AMOUNT_LEN        44
#define G_PAYLOAD_LEN       64
#define G_ADDRESS_TITLE_LEN 32

#define HINT_TITLE_SIZE 32
#define HINT_BODY_SIZE  256

/**
 * Callback to reuse action with approve/reject in step FLOW.
 */
typedef void (*action_validate_cb)(bool);

/**
 * Display address on the device and ask confirmation to export.
 *
 * @return 0 if success, negative integer otherwise.
 *
 */
int ui_display_address(uint8_t flags);

/**
 * Display transaction information on the device and ask confirmation to sign.
 *
 * @return 0 if success, negative integer otherwise.
 *
 */
int ui_display_transaction(void);

/**
 * Display address proof information on the device and ask confirmation to sign.
 *
 * @return 0 if success, negative integer otherwise.
 *
 */
int ui_display_proof(uint8_t flags);

/**
 * Display custom data information on the device and ask confirmation to sign.
 *
 * @return 0 if success, negative integer otherwise.
 *
 */
int ui_display_sign_data(void);
