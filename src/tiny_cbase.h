#ifndef TINY_CBASE_H
#define TINY_CBASE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// --- Feature flags ---
//
#ifndef TINY_CBASE_ENABLE_BASE16
#define TINY_CBASE_ENABLE_BASE16 1
#endif
#ifndef TINY_CBASE_ENABLE_BASE32
#define TINY_CBASE_ENABLE_BASE32 1
#endif
#ifndef TINY_CBASE_ENABLE_BASE58
#define TINY_CBASE_ENABLE_BASE58 1
#endif
#ifndef TINY_CBASE_ENABLE_BASE64
#define TINY_CBASE_ENABLE_BASE64 1
#endif
#ifndef TINY_CBASE_ENABLE_BASE85
#define TINY_CBASE_ENABLE_BASE85 1
#endif

#ifndef BASE_TRUNCATE_ON_NULL
#define BASE_TRUNCATE_ON_NULL 0
#endif

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

typedef enum {
    // --- Base16 / Hex ---
    BASE16_UPPER       = 0x01,  
    BASE16_LOWER       = 0x02,  
    BASE16_DECODE      = 0x04,

    // --- Base32 ---
    BASE32_ENC         = 0x10,
    BASE32_DEC         = 0x20,
    BASE32_ENC_NOPAD   = 0x40,
    BASE32_DEC_NOPAD   = 0x80,

    // --- Base58 ---
    BASE58_ENC         = 0x100,
    BASE58_DEC         = 0x200,

    // --- Base64 ---
    BASE64_STD_ENC        = 0x400,
    BASE64_STD_DEC        = 0x800,
    BASE64_URL_ENC        = 0x1000,
    BASE64_URL_DEC        = 0x2000,
    BASE64_URL_ENC_NOPAD  = 0x4000,
    BASE64_URL_DEC_NOPAD  = 0x8000,

    // --- Base85 ---
    BASE85_STD_ENC     = 0x10000,   
    BASE85_STD_DEC     = 0x20000,
    BASE85_EXT_ENC     = 0x40000,   
    BASE85_EXT_DEC     = 0x80000,
    BASE85_Z85_ENC     = 0x100000,  
    BASE85_Z85_DEC     = 0x200000,
    BASE85_IGNORE_WS   = 0x400000
} BASE_Encoding;

//
// --- Function prototypes and Length macros ---
//
#if TINY_CBASE_ENABLE_BASE16
#define BASE16_ENC_LEN(data_len) ((size_t)(data_len)*2+1)
#define BASE16_DEC_LEN(data_len) ((size_t)(data_len)/2)

bool BASE16_Encode(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len, int mode_flags);
bool BASE16_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len);

static FORCE_INLINE bool BASE16_EncodeUpper(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE16_Encode(data, data_len, out_encoded, out_encoded_len, BASE16_UPPER);
}
static FORCE_INLINE bool BASE16_EncodeLower(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE16_Encode(data, data_len, out_encoded, out_encoded_len, BASE16_LOWER);
}
#endif

#if TINY_CBASE_ENABLE_BASE32
#define BASE32_ENC_LEN(data_len) (8*(((size_t)(data_len)+4)/5)+1)
#define BASE32_DEC_LEN(data_len) (5*((size_t)(data_len)/8))

bool BASE32_Encode(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len, int mode_flags);
bool BASE32_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len, int mode_flags);

static FORCE_INLINE bool BASE32_EncodeStd(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE32_Encode(data, data_len, out_encoded, out_encoded_len, BASE32_ENC);
}
static FORCE_INLINE bool BASE32_DecodeStd(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE32_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE32_DEC);
}

static FORCE_INLINE bool BASE32_EncodeStdNoPad(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE32_Encode(data, data_len, out_encoded, out_encoded_len, BASE32_ENC | BASE32_ENC_NOPAD);
}
static FORCE_INLINE bool BASE32_DecodeStdNoPad(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE32_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE32_DEC | BASE32_DEC_NOPAD);
}
#endif

#if TINY_CBASE_ENABLE_BASE58
#define BASE58_ENC_LEN(data_len) ((size_t)((data_len)*138/100+2))
#define BASE58_DEC_LEN(str_len)  ((size_t)((str_len)*733/1000+1))

bool BASE58_Encode(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len);
bool BASE58_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len);
#endif

#if TINY_CBASE_ENABLE_BASE64
#define BASE64_ENC_LEN(data_len) (4*(((size_t)(data_len)+2)/3)+1)
#define BASE64_DEC_LEN(data_len) (3*((size_t)(data_len)/4))

bool BASE64_Encode(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len, int mode_flags);
bool BASE64_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len, int mode_flags);

static FORCE_INLINE bool BASE64_EncodeStd(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE64_Encode(data, data_len, out_encoded, out_encoded_len, BASE64_STD_ENC);
}
static FORCE_INLINE bool BASE64_DecodeStd(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE64_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE64_STD_DEC);
}

static FORCE_INLINE bool BASE64_EncodeUrl(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE64_Encode(data, data_len, out_encoded, out_encoded_len, BASE64_URL_ENC);
}
static FORCE_INLINE bool BASE64_DecodeUrl(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE64_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE64_URL_DEC);
}

static FORCE_INLINE bool BASE64_EncodeUrlNoPad(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE64_Encode(data, data_len, out_encoded, out_encoded_len, BASE64_URL_ENC | BASE64_URL_ENC_NOPAD);
}
static FORCE_INLINE bool BASE64_DecodeUrlNoPad(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE64_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE64_URL_DEC | BASE64_URL_DEC_NOPAD);
}

#endif

#if TINY_CBASE_ENABLE_BASE85
#define BASE85_ENC_LEN(data_len) (((size_t)(data_len)+3)/4*5+1)
#define BASE85_DEC_LEN(data_len) ((size_t)(data_len)/5*4+4)

bool BASE85_Encode(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len, int mode_flags);
bool BASE85_Decode(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len, int mode_flags);

static FORCE_INLINE bool BASE85_EncodeStd(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE85_Encode(data, data_len, out_encoded, out_encoded_len, BASE85_STD_ENC);
}
static FORCE_INLINE bool BASE85_DecodeStd(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE85_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE85_STD_DEC);
}

static FORCE_INLINE bool BASE85_EncodeExt(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE85_Encode(data, data_len, out_encoded, out_encoded_len, BASE85_EXT_ENC);
}
static FORCE_INLINE bool BASE85_DecodeExt(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE85_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE85_EXT_DEC);
}

static FORCE_INLINE bool BASE85_EncodeZ85(const uint8_t *data, size_t data_len, char *out_encoded, size_t *out_encoded_len) {
    return BASE85_Encode(data, data_len, out_encoded, out_encoded_len, BASE85_Z85_ENC);
}
static FORCE_INLINE bool BASE85_DecodeZ85(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE85_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE85_Z85_DEC);
}

static FORCE_INLINE bool BASE85_CheckZ85Len(size_t len) {
    return (len % 4) == 0;
}

// Optionally ignore whitespace automatically
static FORCE_INLINE bool BASE85_DecodeStdIgnoreWS(const char *encoded_data, size_t encoded_len, uint8_t *out_decoded, size_t *out_decoded_len) {
    return BASE85_Decode(encoded_data, encoded_len, out_decoded, out_decoded_len, BASE85_STD_DEC | BASE85_IGNORE_WS);
}
#endif

static FORCE_INLINE size_t BASE_GetEncodeLen(size_t data_len, uint32_t mode) {
    if (!data_len) return 0;

#if TINY_CBASE_ENABLE_BASE16
    if (mode & (BASE16_UPPER | BASE16_LOWER)) {
        return BASE16_ENC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE32
    if (mode & BASE32_ENC) {
        return BASE32_ENC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE58
    if (mode & BASE58_ENC) {
        return BASE58_ENC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE64
    if (mode & (BASE64_STD_ENC | BASE64_URL_ENC | BASE64_URL_ENC_NOPAD)) {
        return BASE64_ENC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE85
    if (mode & (BASE85_STD_ENC | BASE85_EXT_ENC | BASE85_Z85_ENC)) {
        return BASE85_ENC_LEN(data_len);
    }
#endif

    return 0; // unknown mode
}

static FORCE_INLINE size_t BASE_GetDecodeLen(size_t data_len, uint32_t mode) {
    if (!data_len) return 0;

#if TINY_CBASE_ENABLE_BASE16
    if (mode & BASE16_DECODE) {
        return BASE16_DEC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE32
    if (mode & BASE32_DEC) {
        return BASE32_DEC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE58
    if (mode & BASE58_DEC) {
        return BASE58_DEC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE64
    if (mode & (BASE64_STD_DEC | BASE64_URL_DEC | BASE64_URL_DEC_NOPAD)) {
        return BASE64_DEC_LEN(data_len);
    }
#endif

#if TINY_CBASE_ENABLE_BASE85
    if (mode & (BASE85_STD_DEC | BASE85_EXT_DEC | BASE85_Z85_DEC)) {
        return BASE85_DEC_LEN(data_len);
    }
#endif

    return 0; // unknown mode
}

#ifdef __cplusplus
}
#endif

#endif // TINY_CBASE_H