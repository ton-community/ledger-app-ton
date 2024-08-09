#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/mybuffer.h"

bool deserialize_proof(buffer_t *cdata, uint8_t flags);
