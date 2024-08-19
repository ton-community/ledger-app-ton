#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

/**
 * Encode input bytes in base 64.
 *
 * @see https://datatracker.ietf.org/doc/html/rfc4648
 *
 * @param[in]  in
 *   Pointer to input byte buffer.
 * @param[in]  in_len
 *   Length of the input byte buffer.
 * @param[out] out
 *   Pointer to output string buffer.
 * @param[in]  out_len
 *   Maximum length to write in output byte buffer.
 *
 * @return number of bytes encoded, -1 otherwise.
 *
 */
int base64_encode(const uint8_t *data, size_t data_length, char *out, size_t out_len);

/**
 * Decode a Base64URL encoded string.
 *
 * This function decodes a Base64URL encoded string into its original binary form.
 * The Base64URL encoding is similar to standard Base64, but uses different characters
 * for URL-safe encoding, as defined in RFC 4648.
 *
 * @see https://datatracker.ietf.org/doc/html/rfc4648
 *
 * @param[in]  src
 *   Pointer to the input Base64URL encoded string.
 * @param[in]  src_len
 *   The length of the Base64URL encoded string.
 * @param[out] dest
 *   Pointer to the output buffer where the decoded bytes will be stored.
 * @param[in]  dest_len
 *   The maximum size of the output buffer.
 *
 * @return The number of bytes successfully decoded and stored in the output buffer,
 *         or -1 if an error occurs
 *
 */
int base64url_decode(const char *src, size_t src_len, uint8_t *dest, size_t dest_len);

/**
 * Convert a Base64 encoded string to a Base64URL encoded string.
 *
 * This function converts a given Base64 encoded string to a Base64URL encoded
 * string by replacing certain characters and removing any padding characters ('=')
 * at the end of the string. The result is stored in the provided destination buffer.
 *
 * Base64URL encoding is defined in RFC 4648 and is a URL-safe variant of Base64 encoding,
 * where '+' is replaced with '-' and '/' is replaced with '_'.
 *
 * @see https://datatracker.ietf.org/doc/html/rfc4648
 *
 * @param[in]  src
 *   Pointer to the buffer containing the Base64 encoded string.
 * @param[in]  src_len
 *   Length of the Base64 encoded string in the source buffer.
 * @param[out] dest
 *   Pointer to the buffer where the Base64URL encoded string will be stored.
 *   The buffer should be large enough to hold the result, which is typically
 *   the same size as the source buffer.
 *
 * @param[in]  dest_len
 *   Maximum length of the destination buffer.
 *
 * @return Number of characters written to the destination buffer, excluding the null terminator,
 *         or -1 if the destination buffer is too small.
 *
 */
int base64_to_base64url(const char *src, size_t src_len, char *dest, size_t dest_len);
