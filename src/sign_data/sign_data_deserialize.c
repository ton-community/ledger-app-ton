#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../common/buffer.h"
#include "../common/encoding.h"
#include "../common/bits.h"
#include "../common/cell.h"
#include "../common/types.h"
#include "../constants.h"
#include "../types.h"

#define PLAINTEXT_REQUEST          0x754bf91b
#define APP_DATA_REQUEST           0x54b58535
#define MAX_PLAINTEXT_LENGTH       188
#define MAX_APP_DATA_DOMAIN_LENGTH 126  // max allowed domain len as per TON DNS spec

#define SAFE(RES)     \
    if (!RES) {       \
        return false; \
    }

int roffset(uint8_t* data, size_t data_len, uint8_t c) {
    for (int i = data_len - 1; i >= 0; i--) {
        if (data[i] == c) {
            return i;
        }
    }
    return -1;
}

void encode_domain(BitString_t* self, uint8_t* domain, size_t domain_len) {
    size_t cur_len = domain_len;
    int doffset;
    while ((doffset = roffset(domain, cur_len, '.')) >= 0) {
        BitString_storeBuffer(self, &domain[doffset + 1], cur_len - doffset - 1);
        BitString_storeUint(self, 0, 8);
        cur_len = doffset;
    }
    BitString_storeBuffer(self, domain, cur_len);
    BitString_storeUint(self, 0, 8);
}

void encode_text(BitString_t* self,
                 uint8_t* data,
                 size_t data_len,
                 CellRef_t* out_ref,
                 bool* out_has_ref) {
    uint8_t storeMax = (1023 - self->data_cursor) / 8;
    if (data_len > storeMax) {
        BitString_t inner;
        CellRef_t innerRef;
        bool innerHasRef;
        CellRef_t selfRef;
        BitString_init(&inner);
        encode_text(&inner, &data[storeMax], data_len - storeMax, &innerRef, &innerHasRef);
        hash_Cell(&inner, &innerRef, innerHasRef ? 1 : 0, &selfRef);
        BitString_storeBuffer(self, data, storeMax);
        *out_ref = selfRef;
        *out_has_ref = true;
    } else {
        BitString_storeBuffer(self, data, data_len);
        *out_has_ref = false;
    }
}

bool sign_data_deserialize(buffer_t* buf, sign_data_ctx_t* ctx) {
    SAFE(buffer_read_u32(buf, &ctx->schema_crc, BE));
    SAFE(buffer_read_u64(buf, &ctx->timestamp, BE));

    BitString_t bits;
    BitString_init(&bits);
    CellRef_t refs[4] = {0};
    int cur_ref = 0;

    switch (ctx->schema_crc) {
        case PLAINTEXT_REQUEST: {
            size_t len = buffer_remaining(buf);
            if (len > MAX_PLAINTEXT_LENGTH) {
                return false;
            }
            uint8_t* data;
            SAFE(buffer_read_ref(buf, &data, len));
            SAFE(check_ascii(data, len));
            add_hint_text(&ctx->hints, "Text", (char*) data, len);
            bool has_ref;
            encode_text(&bits, data, len, &refs[cur_ref], &has_ref);
            if (has_ref) {
                cur_ref++;
            }
            break;
        }
        case APP_DATA_REQUEST: {
            bool has_address;
            SAFE(buffer_read_bool(buf, &has_address));
            if (has_address) {
                address_t addr;
                SAFE(buffer_read_address(buf, &addr));
                add_hint_address(&ctx->hints, "Contract address", addr, true);
                BitString_storeBit(&bits, 1);
                BitString_storeAddress(&bits, addr.chain, addr.hash);
            } else {
                BitString_storeBit(&bits, 0);
            }

            bool has_domain;
            SAFE(buffer_read_bool(buf, &has_domain));
            if (!has_address && !has_domain) {
                return false;
            }
            if (has_domain) {
                uint8_t domain_len;
                SAFE(buffer_read_u8(buf, &domain_len));
                if (domain_len > MAX_APP_DATA_DOMAIN_LENGTH) {
                    return false;
                }
                uint8_t* domain;
                SAFE(buffer_read_ref(buf, &domain, domain_len));
                SAFE(check_ascii(domain, domain_len));
                BitString_t inner;
                BitString_init(&inner);
                encode_domain(&inner, domain, domain_len);
                hash_Cell(&inner, NULL, 0, &refs[cur_ref]);
                cur_ref++;
                BitString_storeBit(&bits, 1);
                add_hint_text(&ctx->hints, "App domain", (char*) domain, domain_len);
            } else {
                BitString_storeBit(&bits, 0);
            }

            SAFE(buffer_read_cell_ref(buf, &refs[cur_ref]));
            add_hint_hash(&ctx->hints, "Data hash", refs[cur_ref].hash);
            cur_ref++;

            bool has_ext;
            SAFE(buffer_read_bool(buf, &has_ext));
            if (has_ext) {
                SAFE(buffer_read_cell_ref(buf, &refs[cur_ref]));
                add_hint_hash(&ctx->hints, "Extension hash", refs[cur_ref].hash);
                cur_ref++;
                BitString_storeBit(&bits, 1);
            } else {
                BitString_storeBit(&bits, 0);
            }
            break;
        }
        default: {
            return false;
        }
    }

    CellRef_t out;
    SAFE(hash_Cell(&bits, refs, cur_ref, &out));
    memmove(ctx->cell_hash, out.hash, HASH_LEN);

    return true;
}
