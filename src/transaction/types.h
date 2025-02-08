#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "../constants.h"
#include "../common/types.h"
#include "../common/hints.h"

#define MAX_MEMO_LEN 120

#define TX_INCLUDE_WALLET_OP_BIT 1
#define TX_INCLUDE_EXTRA_CURRENCY_BIT 2

typedef enum {
    PARSING_OK = 0,
    SEQ_PARSING_ERROR = -1,
    TIMEOUT_PARSING_ERROR = -2,
    TO_PARSING_ERROR = -3,
    VALUE_PARSING_ERROR = -4,
    WRONG_LENGTH_ERROR = -5,
    TAG_PARSING_ERROR = -6,
    SEND_MODE_PARSING_ERROR = -7,
    BOUNCE_PARSING_ERROR = -8,
    PAYLOAD_PARSING_ERROR = -9,
    STATE_INIT_PARSING_ERROR = -10,
    HINTS_PARSING_ERROR = -11,
    GENERAL_ERROR = -12,
    EXTRA_CURRENCY_PARSING_ERROR = -13,
} parser_status_e;

typedef struct {
    uint8_t tag;  // tag (1 byte)
    uint32_t subwallet_id;
    uint8_t flags;
    uint32_t seqno;                          // seqno (4 bytes)
    uint32_t timeout;                        // timeout (4 bytes)
    uint8_t value_buf[MAX_VALUE_BYTES_LEN];  // big endian transaction value
    uint8_t value_len;                       // length of transaction value
    uint32_t extra_currency_id;              // extra currency id
    uint8_t extra_currency_amount_buf[MAX_EXTRA_CURRENCY_AMOUNT_BYTES_LEN];  // big endian extra currency amount
    uint8_t extra_currency_amount_len;                                       // length of extra currency amount
    bool bounce;                             // bounce
    uint8_t send_mode;                       // send_mode (1 byte)
    address_t to;                            // receiver
    bool has_state_init;                     // true if state_init exists
    CellRef_t state_init;                    // state_init if exists
    bool has_payload;                        // true if payload exists
    CellRef_t payload;                       // payload if exists
    bool has_hints;                          // true if hints exist
    uint32_t hints_type;                     // hints type if exists
    uint16_t hints_len;                      // hints len if exists
    uint8_t* hints_data;                     // hints data if exists
    bool is_blind;                           // does transaction require blind signing
    HintHolder_t hints;
    char title[32];
    char action[32];
    char recipient[32];
} transaction_t;

bool transaction_include_wallet_op(transaction_t* tx);
bool transaction_include_extra_currency(transaction_t* tx);
