#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../constants.h"
#include "types.h"

enum HintKind {
    SummaryItemNone = 0,  // SummaryItemNone always zero
    SummaryItemAmount,
    SummaryItemString,
    SummaryAddress,
    SummaryHash,
    SummaryNumber,
    SummaryBool,
    SummaryHex,
};

typedef struct {
    const char* string;
    size_t length;
} SizedString_t;

typedef struct {
    char ticker[MAX_TICKER_LEN + 1];
    uint8_t value[MAX_VALUE_BYTES_LEN];
    uint8_t value_len;
    uint8_t decimals;
} Amount_t;

typedef struct {
    address_t address;
    bool bounceable;
} HintAddress_t;

typedef struct {
    uint8_t len;
    uint8_t data[HASH_LEN];
} HintHex_t;

typedef struct {
    const char* title;
    enum HintKind kind;
    union {
        Amount_t amount;
        uint64_t number;
        SizedString_t string;
        uint8_t hash[HASH_LEN];
        HintAddress_t address;
        bool bool_value;
        HintHex_t hex;
    };
} Hint_t;

typedef struct {
    Hint_t hints[MAX_HINTS];
    uint8_t hints_count;
} HintHolder_t;

void add_hint_text(HintHolder_t* hints, const char* title, const char* text, size_t text_len);
void add_hint_hash(HintHolder_t* hints, const char* title, uint8_t* data);
void add_hint_amount(HintHolder_t* hints,
                     const char* title,
                     const char* ticker,
                     uint8_t* value,
                     uint8_t value_len,
                     uint8_t decimals);
void add_hint_address(HintHolder_t* hints, const char* title, address_t address, bool bounceable);
void add_hint_number(HintHolder_t* hints, const char* title, uint64_t number);
void add_hint_bool(HintHolder_t* hints, const char* title, bool value);
void add_hint_hex(HintHolder_t* hints, const char* title, uint8_t* data, uint8_t data_len);

void print_hint(HintHolder_t* hints,
                uint16_t index,
                char* title,
                size_t title_len,
                char* body,
                size_t body_len);
