#pragma once

#include <stdint.h>  // uint*_t
#include <stdbool.h>

#include "types.h"

bool hash_Cell(BitString_t *bits, CellRef_t *refs, uint8_t refs_count, CellRef_t *out);
