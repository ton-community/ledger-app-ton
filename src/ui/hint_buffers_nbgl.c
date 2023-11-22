#ifdef HAVE_NBGL

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "nbgl_use_case.h"

#include "hint_buffers_nbgl.h"

#include "../constants.h"
#include "../common/hints.h"

char g_hint_title_buffer[32 * MAX_HINTS];
char g_hint_buffer[64 * MAX_HINTS];

void print_hints(HintHolder_t *hints, nbgl_layoutTagValue_t *pairs) {
    size_t hint_buffer_offset = 0;
    size_t hint_title_buffer_offset = 0;
    int pairIndex = 0;
    for (uint16_t i = 0; i < hints->hints_count; i++) {
        print_hint(hints,
                   i,
                   &g_hint_title_buffer[hint_title_buffer_offset],
                   sizeof(g_hint_title_buffer) - hint_title_buffer_offset,
                   &g_hint_buffer[hint_buffer_offset],
                   sizeof(g_hint_buffer) - hint_buffer_offset);
        pairs[pairIndex].item = &g_hint_title_buffer[hint_title_buffer_offset];
        pairs[pairIndex].value = &g_hint_buffer[hint_buffer_offset];
        pairIndex++;
        hint_title_buffer_offset +=
            strnlen(&g_hint_title_buffer[hint_title_buffer_offset],
                    sizeof(g_hint_title_buffer) - hint_title_buffer_offset) +
            1;
        hint_buffer_offset += strnlen(&g_hint_buffer[hint_buffer_offset],
                                      sizeof(g_hint_buffer) - hint_buffer_offset) +
                              1;
    }
}

#endif
