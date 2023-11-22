#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

#include <cmocka.h>

#include "common/base64.h"

void test_encode(void **state) {
    static const uint8_t input[] = "The quick brown fox jumps over the lazy dog.";

    char output[sizeof(input)*2] = { 0 };

    size_t enc = base64_encode(input, sizeof(input) - 1 /* null terminator */, output, sizeof(output));
    assert_true(enc > 0);

    static const char expected[] = "VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZy4=";

    assert_memory_equal(output, expected, sizeof(expected));
}

void test_not_enough_mem(void **state) {
    static const uint8_t input[] = "The quick brown fox jumps over the lazy dog.";

    char output[sizeof(input)] = { 0 };

    int enc = base64_encode(input, sizeof(input) - 1 /* null terminator */, output, sizeof(output));
    assert_false(enc >= 0);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_encode),
        cmocka_unit_test(test_not_enough_mem)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
