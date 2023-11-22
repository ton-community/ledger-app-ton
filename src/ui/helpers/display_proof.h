#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool display_proof(uint8_t flags,
                   char *g_address,
                   size_t g_address_len,
                   char *g_domain,
                   size_t g_domain_len,
                   char *g_address_title,
                   size_t g_address_title_len);
