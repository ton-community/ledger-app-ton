#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

#include <cmocka.h>

#include "common/crc16.h"

void test_crc16(void **state) {
    uint8_t input[16] = { 0 };

    for (int i = 0; i < sizeof(input); i++) {
        input[i] = i;
    }

    uint16_t output = crc16(input, sizeof(input));

    assert_int_equal(output, 20797);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crc16)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
