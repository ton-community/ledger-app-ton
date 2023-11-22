#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "common/encoding.h"

static void test_check_ascii(void **state) {
    (void) state;

    const uint8_t good_ascii[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21};  // Hello!
    const uint8_t bad_ascii[] = {0x32, 0xc3, 0x97, 0x32, 0x3d, 0x34};   // 2×2=4

    assert_true(check_ascii(good_ascii, sizeof(good_ascii)));
    assert_false(check_ascii(bad_ascii, sizeof(bad_ascii)));
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_check_ascii)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
