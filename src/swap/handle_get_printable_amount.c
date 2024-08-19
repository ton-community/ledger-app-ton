#ifdef HAVE_SWAP

#include "handle_swap_sign_transaction.h"
#include "swap.h"
#include "os.h"
#include "format_bigint.h"
#include "constants.h"

/* Set empty printable_amount on error, printable amount otherwise */
void swap_handle_get_printable_amount(get_printable_amount_parameters_t* params) {
    PRINTF("coin_configuration %.*H\n",
           params->coin_configuration_length,
           params->coin_configuration);
    PRINTF("amount %.*H\n", params->amount_length, params->amount);
    uint8_t decimals;
    char ticker[MAX_SWAP_TOKEN_LENGTH] = {0};

    PRINTF("Inside Ton swap_handle_get_printable_amount\n");

    // If the amount is a fee, its value is nominated in TON even if we're doing an TRC20 swap
    // If there is no coin_configuration, consider that we are doing a TON swap
    if (params->is_fee || params->coin_configuration == NULL) {
        memcpy(ticker, "TON", sizeof("TON"));
        decimals = EXPONENT_SMALLEST_UNIT;
    } else {
        if (!swap_parse_config(params->coin_configuration,
                               params->coin_configuration_length,
                               ticker,
                               sizeof(ticker),
                               &decimals)) {
            PRINTF("Fail to parse coin_configuration\n");
            goto error;
        }
    }

    if (!amountToString(params->amount,
                        params->amount_length,
                        decimals,
                        ticker,
                        params->printable_amount,
                        sizeof(params->printable_amount))) {
        PRINTF("print_amount failed\n");
        goto error;
    }

    PRINTF("Amount %s\n", params->printable_amount);
    return;

error:
    memset(params->printable_amount, '\0', sizeof(params->printable_amount));
}

#endif  // HAVE_SWAP
