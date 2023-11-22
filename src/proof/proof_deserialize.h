#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../common/buffer.h"

bool deserialize_proof(buffer_t *cdata, uint8_t flags);
