#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "hints.h"

#include "types.h"
#include "base64.h"
#include "format_bigint.h"
#include "format_address.h"
#include "format.h"
#include "common/myformat.h"

void add_hint_text(HintHolder_t* hints, const char* title, const char* text, size_t text_len) {
    // Configure
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryItemString;
    hints->hints[hints->hints_count].string.string = text;
    hints->hints[hints->hints_count].string.length = text_len;

    // Next
    hints->hints_count++;
}

void add_hint_hash(HintHolder_t* hints, const char* title, uint8_t* data) {
    // Configure
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryHash;
    memmove(hints->hints[hints->hints_count].hash, data, HASH_LEN);

    // Next
    hints->hints_count++;
}

void add_hint_hex(HintHolder_t* hints, const char* title, uint8_t* data, uint8_t data_len) {
    // Configure
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryHex;
    memmove(hints->hints[hints->hints_count].hex.data,
            data,
            data_len > HASH_LEN ? HASH_LEN : data_len);
    hints->hints[hints->hints_count].hex.len = data_len;

    // Next
    hints->hints_count++;
}

void add_hint_amount(HintHolder_t* hints,
                     const char* title,
                     const char* ticker,
                     uint8_t* value,
                     uint8_t value_len,
                     uint8_t decimals) {
    // Configure
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryItemAmount;
    size_t ticker_len = strnlen(ticker, MAX_TICKER_LEN);
    memmove(hints->hints[hints->hints_count].amount.ticker, ticker, ticker_len);
    hints->hints[hints->hints_count].amount.ticker[ticker_len] = '\0';
    memmove(hints->hints[hints->hints_count].amount.value, value, value_len);
    hints->hints[hints->hints_count].amount.value_len = value_len;
    hints->hints[hints->hints_count].amount.decimals = decimals;

    // Next
    hints->hints_count++;
}

void add_hint_address(HintHolder_t* hints, const char* title, address_t address, bool bounceable) {
    // Configure
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryAddress;
    hints->hints[hints->hints_count].address.address = address;
    hints->hints[hints->hints_count].address.bounceable = bounceable;

    // Next
    hints->hints_count++;
}

void add_hint_number(HintHolder_t* hints, const char* title, uint64_t number) {
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryNumber;
    hints->hints[hints->hints_count].number = number;

    // Next
    hints->hints_count++;
}

void add_hint_bool(HintHolder_t* hints, const char* title, bool value) {
    hints->hints[hints->hints_count].title = title;
    hints->hints[hints->hints_count].kind = SummaryBool;
    hints->hints[hints->hints_count].bool_value = value;

    // Next
    hints->hints_count++;
}

int print_string(const char* in, char* out, size_t out_length) {
    strncpy(out, in, out_length);
    int rc = (out[--out_length] != '\0');
    if (rc) {
        /* ensure the output is NUL terminated */
        out[out_length] = '\0';
        if (out_length != 0) {
            /* signal truncation */
            out[out_length - 1] = '~';
        }
    }
    return rc;
}

int print_sized_string(const SizedString_t* string, char* out, size_t out_length) {
    size_t len = out_length < string->length ? out_length : string->length;
    strncpy(out, string->string, len);
    if (string->length < out_length) {
        out[string->length] = '\0';
        return 0;
    } else {
        out[--out_length] = '\0';
        if (out_length != 0) {
            /* signal truncation */
            out[out_length - 1] = '~';
        }
        return 1;
    }
}

void print_hint(HintHolder_t* hints,
                uint16_t index,
                char* title,
                size_t title_len,
                char* body,
                size_t body_len) {
    Hint_t hint = hints->hints[index];

    // Title
    print_string(hint.title, title, title_len);

    // Body
    if (hint.kind == SummaryItemString) {
        print_sized_string(&hint.string, body, body_len);
    } else if (hint.kind == SummaryHash) {
        base64_encode(hint.hash, HASH_LEN, body, body_len);
    } else if (hint.kind == SummaryItemAmount) {
        amountToString(hint.amount.value,
                       hint.amount.value_len,
                       hint.amount.decimals,
                       hint.amount.ticker,
                       body,
                       body_len);
    } else if (hint.kind == SummaryAddress) {
        uint8_t address[ADDRESS_LEN] = {0};
        address_to_friendly(hint.address.address.chain,
                            hint.address.address.hash,
                            hint.address.bounceable,
                            false,
                            address,
                            sizeof(address));
        memset(body, 0, body_len);
        base64_encode(address, sizeof(address), body, body_len);
    } else if (hint.kind == SummaryNumber) {
        format_u64_my(hint.number, body, body_len);
    } else if (hint.kind == SummaryBool) {
        snprintf(body, body_len, hint.bool_value ? "Yes" : "No");
    } else if (hint.kind == SummaryHex) {
        if (body_len >= 3 + 2 * hint.hex.len) {
            body[0] = '0';
            body[1] = 'x';
            format_hex(hint.hex.data, hint.hex.len, &body[2], body_len - 2);
        }
    } else {
        print_string("<unknown>", body, body_len);
    }
}
