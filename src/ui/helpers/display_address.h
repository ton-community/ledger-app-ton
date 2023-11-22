#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

bool display_address(uint8_t flags,
                     char *g_address,
                     size_t g_address_len,
                     char *g_address_title,
                     size_t g_address_title_len);
