#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "common/bits.h"

static void test_bits(void **state) {
    
    uint8_t expected[21] = {
        0xe3,0x79,0x2f,0x59,0x01,0x0a,0x30,0xf4,0x24,0x05,0x02,0x54,0x0b,0xe4,0x00,0x50,0x2d,0xdd,0xef,0xa0,0x38
    };

    BitString_t bits;
    BitString_init(&bits);

    // Test bits writes
    BitString_storeBit(&bits, 1);
    assert_true(bits.data_cursor == 1);
    BitString_storeBit(&bits, 2);
    assert_true(bits.data_cursor == 2);
    BitString_storeBit(&bits, 3);
    assert_true(bits.data_cursor == 3);
    BitString_storeBit(&bits, 0);
    assert_true(bits.data_cursor == 4);
    BitString_storeBit(&bits, -1);
    assert_true(bits.data_cursor == 5);
    BitString_storeBit(&bits, -2);
    assert_true(bits.data_cursor == 6);
    BitString_storeBit(&bits, 1);
    assert_true(bits.data_cursor == 7);
    BitString_storeBit(&bits, 1);
    assert_true(bits.data_cursor == 8);
    assert_int_equal(bits.data[0], 0xe3);

    // Test uint writes
    BitString_storeUint(&bits, 121, 8);
    BitString_storeUint(&bits, 12121, 16);
    assert_int_equal(bits.data[1], 0x79);
    assert_int_equal(bits.data[2], 0x2f);
    assert_int_equal(bits.data[3], 0x59);

    // Test coins writes
    // 010a30f4240502540be400
    BitString_storeCoins(&bits, 0);
    assert_true(bits.data_cursor == 36);
    BitString_storeCoins(&bits, 10);
    assert_true(bits.data_cursor == 48);
    BitString_storeCoins(&bits, 1000000);
    assert_true(bits.data_cursor == 76);
    BitString_storeCoins(&bits, 10000000000);
    assert_true(bits.data_cursor == 120);
    BitString_storeCoins(&bits, 12312312323);
    assert_true(bits.data_cursor == 164);

    // Finalize
    BitString_finalize(&bits);
    
    // Contents and hash
    assert_memory_equal(bits.data, expected, sizeof(expected));
    
}

static void test_bits_2(void **state) {
    
    uint8_t expected[1] = {
        0x20
    };
    BitString_t bits;
    BitString_init(&bits);
    BitString_storeBit(&bits, 0);
    BitString_storeBit(&bits, 0);
    BitString_finalize(&bits);
    assert_int_equal(bits.data_cursor, 8);
    assert_memory_equal(bits.data, expected, sizeof(expected));
}

static void test_coins_buf(void **state) {
    uint8_t expected[8] = {
        0x07, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xde
    };
    BitString_t bits;
    BitString_init(&bits);
    BitString_storeUint(&bits, 0, 4); // align the coins
    uint8_t coins[7] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xde };
    BitString_storeCoinsBuf(&bits, coins, sizeof(coins));
    BitString_finalize(&bits);
    assert_int_equal(bits.data_cursor, 8 * sizeof(expected));
    assert_memory_equal(bits.data, expected, sizeof(expected));
}

static void test_null_addr(void **state) {
    uint8_t expected[1] = {
        0x20
    };
    BitString_t bits;
    BitString_init(&bits);
    BitString_storeAddressNull(&bits);
    BitString_finalize(&bits);
    assert_int_equal(bits.data_cursor, 8);
    assert_memory_equal(bits.data, expected, sizeof(expected));
}

static void test_addr(void **state) {
    uint8_t expected[34] = { 0 };
    expected[0] = 0x80;
    expected[33] = 0x10;
    uint8_t hash[32] = { 0 };
    BitString_t bits;
    BitString_init(&bits);
    BitString_storeAddress(&bits, 0x00, hash);
    BitString_finalize(&bits);
    assert_int_equal(bits.data_cursor, 8 * sizeof(expected));
    assert_memory_equal(bits.data, expected, sizeof(expected));
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_bits),
        cmocka_unit_test(test_bits_2),
        cmocka_unit_test(test_coins_buf),
        cmocka_unit_test(test_null_addr),
        cmocka_unit_test(test_addr)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
