#pragma once

#include <stdbool.h>  // bool

#include "types.h"

typedef enum {
    TRANSACTION_COMMENT = 0,
    TRANSACTION_TRANSFER_JETTON = 1,
} transaction_hint_type_e;

bool process_hints(transaction_t* tx);
