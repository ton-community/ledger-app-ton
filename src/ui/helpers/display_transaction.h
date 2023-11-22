#pragma once

#include <stdbool.h>
#include <stddef.h>

bool display_transaction(char *g_operation,
                         size_t g_operation_len,
                         char *g_amount,
                         size_t g_amount_len,
                         char *g_address,
                         size_t g_address_len,
                         char *g_payload,
                         size_t g_payload_len,
                         char *g_address_title,
                         size_t g_address_title_len);
