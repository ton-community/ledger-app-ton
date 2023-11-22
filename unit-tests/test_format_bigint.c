#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "common/format_bigint.h"

void test_with_ticker(void **state) {
    uint8_t input[2] = { 0x01, 0x00 };

    char output[16] = { 0 };

    assert_true(amountToString(input, sizeof(input), 0, "TON", output, sizeof(output)));

    static const char expected[] = "TON 256";

    assert_memory_equal(output, expected, sizeof(expected));
}

void test_no_ticker(void **state) {
    uint8_t input[4] = { 0x05, 0xf5, 0xe1, 0x00 };

    char output[16] = { 0 };

    assert_true(amountToString(input, sizeof(input), 9, "", output, sizeof(output)));

    static const char expected[] = "0.1";

    assert_memory_equal(output, expected, sizeof(expected));
}

void test_zero(void **state) {
    uint8_t input[16] = { 0 };

    char output[2] = { 0 };

    assert_true(amountToString(input, sizeof(input), 9, "", output, sizeof(output)));

    static const char expected[] = "0";

    assert_memory_equal(output, expected, sizeof(expected));
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_with_ticker),
        cmocka_unit_test(test_no_ticker),
        cmocka_unit_test(test_zero)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
