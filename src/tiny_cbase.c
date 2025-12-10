/*
 * File: tiny_cbase.c
 * Author: 0xNullll
 * Description: Implementation of the Tiny CBase library.
 *              Provides functions for encoding and decoding binary data
 *              into Base16, Base32, Base58, Base64, and Base85 variants
 *              (Standard, Extended, Z85). Includes support for feature flags,
 *              inline helpers, automatic buffer length calculations,
 *              optional truncation on null, and safe fast operations.
 * License: MIT
 */


#ifndef TINY_CBASE_IMPLEMENTATION
#define TINY_CBASE_IMPLEMENTATION

#include "tiny_cbase.h"

#if TINY_CBASE_ENABLE_BASE16

// Hex encoding table
static const char BASE16_ENC_TABLE_UPPER[] = "0123456789ABCDEF";
static const char BASE16_ENC_TABLE_LOWER[] = "0123456789abcdef";

#define BASE16_MIN '0'
#define BASE16_MAX 'f'

// Base16 reverse lookup table (shifted).
// This table maps ASCII characters '0' (48) to 'f' (102) into Base16 values.
// Indexing: val = BASE32_REV_TABLE[ch - '0']
// Valid:
//  - '0'-'9' -> 0..9
//  - 'A'-'F' -> 10..15
//  - 'a'-'f' -> 10..15
// - Invalid chars are -1
static const int8_t BASE16_REV_TABLE[] = {
    // '0'-'9' (48-57)
     0,1,2,3,4,5,6,7,8,9,
    // ':'- '@' (58-64) -> invalid
    -1,-1,-1,-1,-1,-1,-1,
    // 'A'-'F' (65-70)
    10,11,12,13,14,15,
    // 'G'-'`' (71-96) -> mostly invalid
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,
    // 'a'-'f' (97-102)
    10,11,12,13,14,15
};


bool BASE16_Encode(const uint8_t *raw_data, size_t raw_len, char *out_encoded, size_t *out_encoded_len, int mode_flags) {
    if (!raw_data || raw_len == 0 || !out_encoded || !out_encoded_len) return false;

    const char *table = (mode_flags == BASE16_LOWER || mode_flags == BASE16_UPPER)
                        ? BASE16_ENC_TABLE_LOWER
                        : BASE16_ENC_TABLE_UPPER;

    size_t out_index = 0;

    for (size_t i = 0; i < raw_len; i++) {
        uint8_t byte = raw_data[i];
        out_encoded[out_index++] = table[(byte >> 4) & 0x0F];
        out_encoded[out_index++] = table[byte & 0x0F];
    }

    out_encoded[out_index] = '\0';
    *out_encoded_len = out_index;
    return true;
}

bool BASE16_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    if (!encoded_data || encoded_len == 0 || !out_decoded || !out_decoded_len) return false;

#if BASE_TRUNCATE_ON_NULL
    for (size_t i = 0; i < encoded_len; ++i) {
        if (encoded_data[i] == '\0') {
            encoded_len = i;
            break;
        }
    }
#endif

    if (encoded_len % 2 != 0) return false;

    size_t out_index = 0;
    for (size_t i = 0; i < encoded_len; i += 2) {
        char c1 = encoded_data[i];
        char c2 = encoded_data[i + 1];
        int8_t hi = (c1 >= BASE16_MIN && c1 <= BASE16_MAX) ? BASE16_REV_TABLE[c1 - BASE16_MIN] : -1;
        int8_t lo = (c2 >= BASE16_MIN && c2 <= BASE16_MAX) ? BASE16_REV_TABLE[c2 - BASE16_MIN] : -1;
        if (hi < 0 || lo < 0) return false;

        out_decoded[out_index++] = (uint8_t)((hi << 4) | lo);
    }

    *out_decoded_len = out_index;
    return true;
}

#endif // TINY_CBASE_ENABLE_BASE16

#if TINY_CBASE_ENABLE_BASE32

#define BASE32_PAD_CHAR '='
static const char BASE32_ENC_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

#define BASE32_MIN '2'
#define BASE32_MAX 'Z'

// Base32 reverse lookup table (shifted).
// This table maps ASCII characters '2' (50) to 'Z' (90) into Base32 values.
// Indexing: val = BASE32_REV_TABLE[ch - '2']
// - Valid Base32 chars map to 0..31
//   - 'A'-'Z' -> 0..25
//   - '2'-'7' -> 26..31
// - Invalid chars are -1
// - ignored chars (like '=' or '\n') are -2
static const int8_t BASE32_REV_TABLE[] = {
    // '2'-'7' (50-55)
    26,27,28,29,30,31,
    // '8'-'@' (56-64) → invalid
    -1,-1,-1,-1,-1,-2,-1,-1,-1,
    // 'A'-'Z' (65-90)
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25
};


bool BASE32_Encode(const uint8_t *raw_data, size_t raw_len, char *out_encoded, size_t *out_encoded_len, int mode_flags) {
    if (!raw_data || raw_len == 0 || !out_encoded || !out_encoded_len) return false;

    size_t out_index = 0;
    int no_pad = ((mode_flags & BASE32_ENC_NOPAD) != 0);

    for (size_t i = 0; i < raw_len; i += 5) {
        uint8_t in0 = raw_data[i];
        uint8_t in1 = (i + 1 < raw_len) ? raw_data[i + 1] : 0;
        uint8_t in2 = (i + 2 < raw_len) ? raw_data[i + 2] : 0;
        uint8_t in3 = (i + 3 < raw_len) ? raw_data[i + 3] : 0;
        uint8_t in4 = (i + 4 < raw_len) ? raw_data[i + 4] : 0;

        size_t rem = raw_len - i;
        uint64_t buf = ((uint64_t)in0 << 32) | ((uint64_t)in1 << 24) | ((uint64_t)in2 << 16) |
                       ((uint64_t)in3 << 8) | ((uint64_t)in4);
        size_t total_bits = rem * 8;
        size_t chunks = (total_bits + 4) / 5;

        for (int c = 0; c < 8; c++) {
            if (c < (int)chunks) out_encoded[out_index++] = BASE32_ENC_TABLE[(buf >> (35 - c*5)) & 0x1F];
            else if (!no_pad) out_encoded[out_index++] = BASE32_PAD_CHAR;
        }
    }

    out_encoded[out_index] = '\0';
    *out_encoded_len = out_index;
    return true;
}

bool BASE32_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len, int mode_flags) {
    if (!encoded_data || encoded_len == 0 || !out_decoded || !out_decoded_len) return false;

#if BASE_TRUNCATE_ON_NULL
    // Adjust encoded_len if null terminator appears before
    for (size_t i = 0; i < encoded_len; ++i) {
        if (encoded_data[i] == '\0') {
            encoded_len = i;
            break;
        }
    }
#endif // BASE_TRUNCATE_ON_NULL

    int no_pad = ((mode_flags & BASE32_DEC_NOPAD) != 0);
    if (!no_pad && encoded_len % 8 != 0) return false;

    size_t out_index = 0;
    uint64_t buf;

    for (size_t i = 0; i < encoded_len; i += 8) {
        buf = 0;
        int valid_chars = 0;
        for (int j = 0; j < 8; j++) {
            char c = (i + (size_t)j < encoded_len) ? encoded_data[i + (size_t)j] : BASE32_PAD_CHAR;
            int8_t val = (c == BASE32_PAD_CHAR) ? 0 : (c >= BASE32_MIN && c <= BASE32_MAX) ? BASE32_REV_TABLE[c - BASE32_MIN] : -1;
            if (val < 0) return false;
            buf = (buf << 5) | (uint64_t)val;
            if (c != BASE32_PAD_CHAR) valid_chars++;
        }

        if (valid_chars >= 2) out_decoded[out_index++] = (buf >> 32) & 0xFF;
        if (valid_chars >= 4) out_decoded[out_index++] = (buf >> 24) & 0xFF;
        if (valid_chars >= 5) out_decoded[out_index++] = (buf >> 16) & 0xFF;
        if (valid_chars >= 7) out_decoded[out_index++] = (buf >> 8) & 0xFF;
        if (valid_chars >= 8) out_decoded[out_index++] = buf & 0xFF;
    }

    *out_decoded_len = out_index;
    return true;
}

#endif // TINY_CBASE_ENABLE_BASE32

#if TINY_CBASE_ENABLE_BASE58

#define BASE58_LEADING_ZERO '1'

static const char BASE58_ENC_TABLE[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

#define BASE58_MIN '1'
#define BASE58_MAX 'z'

// Base58 reverse lookup table (shifted).
// This table maps ASCII characters '1' (49) to 'z' (122) into Base58 values.
// Indexing: val = BASE58_REV_TABLE[ch - '1']
// - Valid Base58 chars map to 0..57
//   - '1'-'9'   -> 0..8
//   - 'A'-'H', 'J'-'N', 'P'-'Z' -> 9..32
//   - 'a'-'k', 'm'-'z' -> 33..57
// - Invalid chars are -1
static const int8_t BASE58_REV_TABLE[] = {
     0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,-1,
     9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,22,
    23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,-1,
    33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,47,
    48,49,50,51,52,53,54, 55,56,57
};

bool BASE58_Encode(const uint8_t *raw_data, size_t raw_len, char *out_encoded, size_t *out_encoded_len) {
    if (!raw_data || raw_len == 0 || !out_encoded || !out_encoded_len) return false;

    size_t zcount = 0; // count leading zeros
    while (zcount < raw_len && raw_data[zcount] == 0) zcount++;

    // Approx max size: log(256)/log(58) ≈ 1.38
    size_t size = BASE58_ENC_LEN(raw_len - zcount);
    uint8_t buf[size];
    memset(buf, 0, size);

    size_t i, j, high;
    for (i = zcount, high = size - 1; i < raw_len; ++i, high = j) {
        int val = raw_data[i];
        for (j = size - 1; (j > high) || val; --j) {
            val += 256 * buf[j];
            buf[j] = (uint8_t)(val % 58);
            val /= 58;
            if (!j) break;
        }
    }

    // skip leading zeros in buf
    for (j = 0; j < size && buf[j] == 0; ++j);

    // check output buffer size
    if (*out_encoded_len <= zcount + size - j) {
        *out_encoded_len = zcount + size - j + 1; // required size
        return false;
    }

    // leading '1's for zeros
    for (i = 0; i < zcount; ++i) out_encoded[i] = BASE58_LEADING_ZERO;

    // convert buf -> chars
    for (; j < size; ++i, ++j) {
        out_encoded[i] = BASE58_ENC_TABLE[buf[j]];
    }

    out_encoded[i] = '\0';
    *out_encoded_len = i;
    return true;
}

bool BASE58_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    if (!encoded_data || encoded_len == 0 || !out_decoded || !out_decoded_len) return false;

#if BASE_TRUNCATE_ON_NULL
    // Adjust encoded_len if null terminator appears before
    for (size_t i = 0; i < encoded_data; ++i) {
        if (encoded_data[i] == '\0') {
            encoded_len = i;
            break;
        }
    }
#endif // BASE_TRUNCATE_ON_NULL

    // Count leading '1's -> map to leading zeros
    size_t zcount = 0;
    while (zcount < encoded_len && encoded_data[zcount] == BASE58_LEADING_ZERO) zcount++;

    // Approx max size: 0.733 * digits + 8, gives enough room for intermediate carry/overflow handling.
    size_t size = BASE58_DEC_LEN(encoded_len - zcount);
    uint8_t buf[size];
    memset(buf, 0, size);

    // Convert Base58 digits to big integer in buf
    for (size_t i = zcount; i < encoded_len; ++i) {
        int val = -1;
        char c = encoded_data[i];
        if (c >= BASE58_MIN && c <= BASE58_MAX) val = BASE58_REV_TABLE[c - BASE58_MIN]; // reverse lookup
        if (val < 0) return false; // invalid char

        for (size_t j = size - 1; j != (size_t)-1; --j) {
            val += 58 * buf[j];
            buf[j] = (uint8_t)(val & 0xFF);
            val >>= 8;
        }
    }

    // Skip leading zeros in buf
    size_t j = 0;
    while (j < size && buf[j] == 0) j++;

    size_t out_index = 0;

    // Leading zeros from '1's
    for (size_t i = 0; i < zcount; ++i) out_decoded[out_index++] = 0;

    // Copy remaining bytes
    for (; j < size; ++j) out_decoded[out_index++] = buf[j];

    *out_decoded_len = out_index;
    return true;
}

#endif // TINY_CBASE_ENABLE_BASE58

#if TINY_CBASE_ENABLE_BASE64

#define BASE64_PAD_CHAR '='

static const char BASE64_ENC_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char BASE64_URL_SAFE_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

#define BASE64_MIN '+'
#define BASE64_URL_SAFE_MIN '-'
#define BASE64_MAX 'z'

// Base64 reverse lookup table (shifted).  
// This table maps ASCII characters '+' (43) to 'z' (122) into Base64 values.
// Indexing: val = BASE64_REV_TABLE[ch - '+']
// - Valid Base64 chars map to 0..63
// - Invalid chars are -1
// - ignored chars (like '=' or '\n') are -2
static const int8_t BASE64_REV_TABLE[] = {
    62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,
    -1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,
    11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
    -1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,
    35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,
    50,51
};

// Base64 safe-URL reverse lookup table (shifted).  
// This table maps ASCII characters '-' (45) to 'z' (122) into Base64 safe-URL values.
// Indexing: val = BASE64_REV_TABLE[ch - '-']
// - Valid Base64 chars map to 0..63
// - Invalid chars are -1
// - ignored chars (like '=' or '\n') are -2
static const int8_t BASE64_REV_URL_SAFE_TABLE[] = {
    62,-1,-1,52,53,54,55,56,57,58,59,60,61,
    -1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,
    11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
    -1,-1,-1,-1,63,-1,26,27,28,29,30,31,32,33,34,
    35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,
    50,51
};

bool BASE64_Encode(const uint8_t *raw_data, size_t raw_len, char *out_encoded, size_t *out_encoded_len, int mode_flags) {
    if (!raw_data || raw_len == 0 || !out_encoded || !out_encoded_len) return false;

    bool url_safe = (mode_flags & BASE64_URL_ENC) != 0;
    bool no_pad   = (mode_flags & BASE64_NOPAD_ENC) != 0;

    const char *enc_table = url_safe ? BASE64_URL_SAFE_TABLE : BASE64_ENC_TABLE;
    size_t out_index = 0;

    for (size_t i = 0; i < raw_len; i += 3) {
        uint8_t byte0 = raw_data[i];
        uint8_t byte1 = (i + 1 < raw_len) ? raw_data[i + 1] : 0;
        uint8_t byte2 = (i + 2 < raw_len) ? raw_data[i + 2] : 0;

        uint32_t buf24 = (byte0 << 16) | (byte1 << 8) | byte2;
        size_t remaining = raw_len - i;

        out_encoded[out_index++] = enc_table[(buf24 >> 18) & 0x3F];
        out_encoded[out_index++] = enc_table[(buf24 >> 12) & 0x3F];

        if (remaining > 1) out_encoded[out_index++] = enc_table[(buf24 >> 6) & 0x3F];
        else if (!no_pad) out_encoded[out_index++] = BASE64_PAD_CHAR;

        if (remaining > 2) out_encoded[out_index++] = enc_table[buf24 & 0x3F];
        else if (!no_pad) out_encoded[out_index++] = BASE64_PAD_CHAR;
    }

    out_encoded[out_index] = '\0';
    *out_encoded_len = out_index;
    return true;
}

bool BASE64_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len, int mode_flags) {
    if (!encoded_data || encoded_len == 0 || !out_decoded || !out_decoded_len) return false;

#if BASE_TRUNCATE_ON_NULL
    // Adjust encoded_len if null terminator appears before
    for (size_t i = 0; i < encoded_len; ++i) {
        if (encoded_data[i] == '\0') {
            encoded_len = i;
            break;
        }
    }
#endif // BASE_TRUNCATE_ON_NULL

    bool isStd = (mode_flags & BASE64_STD_DEC) != 0;
    bool isUrlSafe = (mode_flags & BASE64_URL_DEC) != 0;
    bool noPad = (mode_flags & BASE64_NOPAD_DEC) != 0;

    // Only check padding rules if standard Base64 or URL-safe with padding
    if ((isStd || isUrlSafe) && !noPad && (encoded_len % 4 != 0)) {
        return false; // invalid length
    }

    const char start_char = isUrlSafe ? BASE64_URL_SAFE_MIN : BASE64_MIN;
    const int8_t *rev_table = isUrlSafe ? BASE64_REV_URL_SAFE_TABLE : BASE64_REV_TABLE;

    size_t out_index = 0;

    for (size_t i = 0; i < encoded_len; i += 4) {
        uint32_t buf24 = 0;
        int valid_chars = 0;

        for (int j = 0; j < 4; ++j) {
            char c = (i + j < encoded_len) ? encoded_data[i + j] : BASE64_PAD_CHAR;
            int8_t val = (c == BASE64_PAD_CHAR) ? 0 : ((c >= start_char && c <= BASE64_MAX) ? rev_table[c - start_char] : -1);
            if (val < 0) return false;

            buf24 |= ((uint32_t)val << (18 - j * 6));
            if (c != BASE64_PAD_CHAR) valid_chars++;
        }

        if (valid_chars >= 2) out_decoded[out_index++] = (buf24 >> 16) & 0xFF;
        if (valid_chars >= 3) out_decoded[out_index++] = (buf24 >> 8) & 0xFF;
        if (valid_chars >= 4) out_decoded[out_index++] = buf24 & 0xFF;
    }

    *out_decoded_len = out_index;
    return true;
}

#endif // TINY_CBASE_ENABLE_BASE64

#if TINY_CBASE_ENABLE_BASE85


#define ASCII85_ZERO_SHORTCUT 'z'    // shortcut for 0x00000000
#define ASCII85_SPACE_SHORTCUT 'y'   // shortcut for 0x20202020

static const char BASE85_Z85_ENC_TABLE[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

#define BASE85_ASCII85_MIN '!'
#define BASE85_Z85_MIN '!'
#define BASE85_ASCII85_MAX 'u'
#define BASE85_Z85_MAX '}'

// Base85 reverse lookup table (Ascii85).
// This table maps ASCII characters '!' (33) to 'u' (117) into Base85 values.
// Indexing: val = BASE85_ASCII85_REV_TABLE[ch - '!']
// - Valid Ascii85 chars map to 0..84
// - Table is sequential because Ascii85 digits are simply (ch - '!')
static const int8_t BASE85_ASCII85_REV_TABLE[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,    // ! to )
   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,    // * to +
   20, 21, 22, 23, 24, 25, 26, 27, 28, 29,    // , to 2
   30, 31, 32, 33, 34, 35, 36, 37, 38, 39,    // 3 to <
   40, 41, 42, 43, 44, 45, 46, 47, 48, 49,    // = to ?
   50, 51, 52, 53, 54, 55, 56, 57, 58, 59,    // @ to I
   60, 61, 62, 63, 64, 65, 66, 67, 68, 69,    // J to R
   70, 71, 72, 73, 74, 75, 76, 77, 78, 79,    // S to [
   80, 81, 82, 83, 84                         // \ to u
};

// Base85 reverse lookup table (Z85).
// This table maps ASCII characters '!' (33) to '}' (125) into Z85 values.
// Indexing: val = BASE85_Z85_REV_TABLE[ch - '!']
// - Valid Z85 chars map to 0..84
// - Table is not sequential like Ascii85; it follows the Z85 specification order
static const int8_t BASE85_Z85_REV_TABLE[] = {
    68, -1, 84, 83, 82, 72, -1, 75, 76, 70, 65, -1, 63, 62, 69, // offsets 0-14
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // '0'-'9' offsets 15-24
    64, -1, 73, 66, 74, 71, 81,   // offsets 25-31
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // 'A'-'Z' offsets 32-57
    77, -1, 78, 67, -1, -1, // punctuation offsets 58-63
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, // 'a'-'z' offsets 64-89
    79, -1, 80  // '{', '}', offsets 90-92
};

static FORCE_INLINE void write_u32_be(uint8_t *out_decoded, uint32_t value) {
    // Manual big-endian
    out_decoded[0] = (uint8_t)((value >> 24) & 0xFF);
    out_decoded[1] = (uint8_t)((value >> 16) & 0xFF);
    out_decoded[2] = (uint8_t)((value >> 8) & 0xFF);
    out_decoded[3] = (uint8_t)(value & 0xFF);
}

static FORCE_INLINE uint32_t read_u32_be(const uint8_t *in) {
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) | ((uint32_t)in[2] << 8) | in[3];
}

// --- Encode Base85 / Z85 ---
bool BASE85_Encode(const uint8_t *encoded_data, size_t encoded_len, char *out_decoded, size_t *out_decoded_len, int mode_flags) {
    if (!encoded_data || !out_decoded || !out_decoded_len) return false;

    bool isZ85 = (mode_flags & BASE85_Z85_ENC) != 0;
    bool useExt = (mode_flags & BASE85_EXT_ENC) != 0;

    if (isZ85) {
        // Z85 requires input length multiple of 4
        if (encoded_len % 4 != 0) {
            return false;  // cannot encode
        }
    }

    size_t index = 0;
    size_t i = 0;

    // Full 4-byte blocks
    for (; i + 3 < encoded_len; i += 4) {
        uint32_t buf = read_u32_be(encoded_data + i);

        // Shortcuts
        if (!isZ85 && buf == 0) {
            out_decoded[index++] = 'z';
            continue;
        }
        
        if (!isZ85 && useExt && buf == 0x20202020) {
            out_decoded[index++] = 'y';
            continue;
        }

        uint32_t tmp = buf;
        char enc[5];

        for (int j = 4; j >= 0; j--) {
            enc[j] = (char)(tmp % 85);
            tmp /= 85;
        }

        for (int j = 0; j < 5; j++) {
            if (isZ85) enc[j] = BASE85_Z85_ENC_TABLE[(unsigned char)enc[j]];
            else enc[j] += 33;
        }

        memcpy(out_decoded + index, enc, 5);
        index += 5;
    }

    // Partial tail
    size_t tail = encoded_len - i;
    if (!isZ85 && tail > 0) {
        uint32_t buf = 0;
        for (size_t j = 0; j < tail; j++) buf |= (uint32_t)encoded_data[i + j] << (24 - 8 * j);

        uint32_t tmp = buf;
        char enc[5];
        for (int j = 4; j >= 0; j--) {
            enc[j] = (char)(tmp % 85);
            tmp /= 85;
        }

        for (int j = 0; j < 5; j++) {
            enc[j] += 33;
        }

        for (size_t j = 0; j < tail + 1; j++) {
            out_decoded[index++] = enc[j];
        }
    }

    out_decoded[index] = '\0';
    *out_decoded_len = index;
    return true;
}

// // --- Decode Base85 / Z85 ---
bool BASE85_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len, int mode_flags) {
    if (!encoded_data || encoded_len == 0 || !out_decoded || !out_decoded_len) return false;

#if BASE_TRUNCATE_ON_NULL
    // Adjust encoded_len if null terminator appears before
    for (size_t i = 0; i < encoded_len; ++i) {
        if (encoded_data[i] == '\0') {
            encoded_len = i;
            break;
        }
    }
#endif // BASE_TRUNCATE_ON_NULL

    bool isZ85 = (mode_flags & BASE85_Z85_DEC) != 0;
    bool useExt = (mode_flags & BASE85_EXT_DEC) != 0;

    if (isZ85) {
        // Z85 decoding requires input length multiple of 5 characters
        if (encoded_len % 5 != 0) {
            return false;  // invalid input length for Z85
        }
    }

    const int8_t *rev_table = isZ85 ? BASE85_Z85_REV_TABLE : BASE85_ASCII85_REV_TABLE;
    char min_char = isZ85 ? BASE85_Z85_MIN : BASE85_ASCII85_MIN;
    char max_char = isZ85 ? BASE85_Z85_MAX : BASE85_ASCII85_MAX;

    size_t index = 0;
    uint32_t value = 0;
    int count = 0;

    for (size_t i = 0; i < encoded_len; i++) {
        char c = encoded_data[i];

        // Ignore whitespace if flag is set
        if ((mode_flags & BASE85_IGNORE_WS) && isspace((unsigned char)c)) continue;

        // Shortcuts (ASCII85 only)
        if (!isZ85 && c == ASCII85_ZERO_SHORTCUT && count == 0) { 
            write_u32_be(out_decoded + index, 0); 
            index += 4; 
            continue; 
        }
        if (!isZ85 && useExt && c == ASCII85_SPACE_SHORTCUT && count == 0) { 
            write_u32_be(out_decoded + index, 0x20202020); 
            index += 4; 
            continue; 
        }

        // Convert char -> value
        int val = -1;
        if ((unsigned char)c >= (unsigned char)min_char && (unsigned char)c <= (unsigned char)max_char) {
            val = rev_table[(unsigned char)c - (unsigned char)min_char];
        }
        if (val < 0) {
            return false; // invalid character
        }

        value = value * 85 + (uint32_t)val;
        count++;

        if (count == 5) {
            write_u32_be(out_decoded + index, value);
            index += 4;
            value = 0;
            count = 0;
        }
    }

    // Handle partial final block (ASCII85 only)
    if (!isZ85 && count > 0) {
        for (int j = count; j < 5; j++) {
            value = value * 85 + 84; // pad with 'u'
        }
        for (int j = 0; j < count - 1; j++) {
            out_decoded[index + (size_t)j] = (uint8_t)(value >> (24 - j * 8));
        }
        index += (size_t)(count - 1);
    } else if (isZ85 && count != 0) {
        // Z85 cannot have partial blocks
        return false;
    }

    *out_decoded_len = index;
    return true;
}

#endif // TINY_CBASE_ENABLE_BASE85


#endif // TINY_CBASE_IMPLEMENTATION