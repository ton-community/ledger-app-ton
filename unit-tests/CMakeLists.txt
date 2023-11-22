cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
        VERSION 0.1
	      DESCRIPTION "Unit tests for Ledger Nano application"
        LANGUAGES C)


# guard against bad build-type strings
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -g -O0 --coverage")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

add_compile_definitions(TEST)

include_directories(../src)

add_executable(test_bip32 test_bip32.c)
add_executable(test_buffer test_buffer.c)
add_executable(test_write test_write.c)
add_executable(test_apdu_parser test_apdu_parser.c)
add_executable(test_bits test_bits.c)
add_executable(test_base64 test_base64.c)
add_executable(test_crc16 test_crc16.c)
add_executable(test_format_bigint test_format_bigint.c)
add_executable(test_encoding test_encoding.c)

add_library(bip32 SHARED ../src/common/bip32.c)
add_library(buffer SHARED ../src/common/buffer.c)
add_library(read SHARED ../src/common/read.c)
add_library(write SHARED ../src/common/write.c)
add_library(bits SHARED ../src/common/bits.c)
add_library(apdu_parser SHARED ../src/apdu/parser.c)
add_library(base64 SHARED ../src/common/base64.c)
add_library(crc16 SHARED ../src/common/crc16.c)
add_library(int256 SHARED ../src/common/int256.c)
add_library(format_bigint SHARED ../src/common/format_bigint.c)
add_library(encoding SHARED ../src/common/encoding.c)
add_library(format_address SHARED ../src/common/format_address.c)
add_library(strlcpy_impl SHARED strlcpy_impl.c)

target_link_libraries(int256 strlcpy_impl)
target_link_libraries(format_bigint int256)
target_link_libraries(buffer bip32)
target_link_libraries(format_address crc16)

target_link_libraries(test_bip32 PUBLIC cmocka gcov bip32 read)
target_link_libraries(test_buffer PUBLIC cmocka gcov buffer bip32 write read)
target_link_libraries(test_write PUBLIC cmocka gcov write)
target_link_libraries(test_bits PUBLIC cmocka gcov bits)
target_link_libraries(test_apdu_parser PUBLIC cmocka gcov apdu_parser)
target_link_libraries(test_base64 PUBLIC cmocka gcov base64)
target_link_libraries(test_crc16 PUBLIC cmocka gcov crc16)
target_link_libraries(test_format_bigint PUBLIC cmocka gcov format_bigint)
target_link_libraries(test_encoding PUBLIC cmocka gcov encoding)

add_test(test_bip32 test_bip32)
add_test(test_buffer test_buffer)
add_test(test_write test_write)
add_test(test_bits test_bits)
add_test(test_apdu_parser test_apdu_parser)
add_test(test_base64 test_base64)
add_test(test_crc16 test_crc16)
add_test(test_format_bigint test_format_bigint)
add_test(test_encoding test_encoding)
