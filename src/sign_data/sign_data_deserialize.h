#pragma once

#include <stdbool.h>

#include "../common/buffer.h"
#include "../types.h"

bool sign_data_deserialize(buffer_t *buf, sign_data_ctx_t *ctx);
