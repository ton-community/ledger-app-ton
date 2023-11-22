#pragma once

#include <stdbool.h>

#include "nbgl_use_case.h"

#include "../common/hints.h"
#include "../constants.h"

extern char g_hint_title_buffer[32 * MAX_HINTS];
extern char g_hint_buffer[64 * MAX_HINTS];

void print_hints(HintHolder_t *hints, nbgl_layoutTagValue_t *pairs);
