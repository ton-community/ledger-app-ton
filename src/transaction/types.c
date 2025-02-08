#include "types.h"

bool transaction_include_wallet_op(transaction_t* tx) {
    return (tx->flags & TX_INCLUDE_WALLET_OP_BIT) != 0;
}

bool transaction_include_extra_currency(transaction_t* tx) {
    return (tx->flags & TX_INCLUDE_EXTRA_CURRENCY_BIT) != 0;
}
