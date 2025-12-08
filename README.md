# Tiny CBase

## Description

**Tiny CBase** is a lightweight C library (header + source) for encoding and decoding binary data into various Base formats. It supports Base16, Base32, Base58, Base64, and Base85 (including Z85 and extended variants). The library is designed for performance and easy integration, providing inline helper functions, automatic length calculations, and optional whitespace handling.

---

## Features

- Header-only and portable across C/C++ compilers.
- **Supported Encodings:**
  - **Base16**: Uppercase and lowercase hex
  - **Base32**: Standard and no-padding variants
  - **Base58**: Bitcoin-style Base58
  - **Base64**: Standard, URL-safe, and no-padding variants
  - **Base85**: Standard, Extended, Z85, with optional whitespace ignoring
- Inline wrappers for simplified usage.
- Compile-time feature flags to include/exclude specific encodings.
- Automatic buffer size calculation with `BASE_GetEncodeLen` and `BASE_GetDecodeLen`.
- Safe and fast with optional truncation (BASE_TRUNCATE_ON_NULL) and always-inline functions.

---

# Configurable Feature Flags

The library allows enabling or disabling specific Base encodings. By default, all are enabled.

Flags are defined inside the header:

```c
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
```

> ⚠️ **Note:** Defining these macros before including the header may not always work. Recommended ways:

1. Update the flags directly inside the header.
2. Use compiler `-D` flags, for example:

```bash
gcc -DTINY_CBASE_ENABLE_BASE16=1 -DTINY_CBASE_ENABLE_BASE32=0 tiny_cbase.c test_cbase.c -o test_cbase
```

This approach allows you to selectively enable or disable any encoding variant at compile time, giving you full control over which encoders/decoders are included in your build.

---

## Usage

### Encoding Example

```c
#include "tiny_cbase.h"
#include <stdio.h>
#include <string.h>

int main() {
    const char *input = "hello world";
    char encoded[128];
    size_t enc_len = sizeof(encoded);

    if (BASE64_EncodeStd((const uint8_t*)input, strlen(input), encoded, &enc_len)) {
        encoded[enc_len] = '\0'; // Null-terminate
        printf("Encoded: %s\n", encoded);
    } else {
        printf("Encoding failed.\n");
    }

    return 0;
}
```

### Decoding Example

```c
uint8_t decoded[128];
size_t dec_len = sizeof(decoded);

if (BASE64_DecodeStd(encoded, enc_len, decoded, &dec_len)) {
    decoded[dec_len] = '\0';
    printf("Decoded: %s\n", decoded);
} else {
    printf("Decoding failed.\n");
}
```

### Z85 Input Check

```c
if (!BASE85_CheckZ85Len(dec_len)) {
    printf("Z85 input length must be a multiple of 4.\n");
}
```

## Raw Encode/Decode Function Flags

Tiny CBase uses a unified bit-flag system to configure all raw encode/decode functions.  
Each flag falls into one of three categories:

- **Functional Flags** – change encoding/decoding behavior (alphabet, padding, whitespace, etc.)  
- **Length-Helper Flags** – consulted by `BASE_GetEncodeLen()` / `BASE_GetDecodeLen()` to compute required buffer sizes  
- **Selector / Clarity Flags** – used by wrappers for consistency; do not affect encoding logic

---

## 🔢 Base16 Flags

| Flag | Meaning  | Affects Encoding?                   | Used by Length Helpers?        |
|------|----------|-------------------------------------|--------------------------------|
| `BASE16_UPPER`  | Encode using uppercase hex alphabet | ✔️ Yes               | ✔️ Yes |
| `BASE16_LOWER`  | Encode using lowercase hex alphabet | ✔️ Yes               | ✔️ Yes |
| `BASE16_DECODE` | Decode-mode selector                | ❌ No runtime effect | ✔️ Yes |

> `BASE16_DECODE` does **not** change decoding logic — only signals the raw API that the operation is decoding.

---

## 🔤 Base32 Flags

| Flag               | Meaning                          | Affects Encoding? | Used by Length Helpers? |
|--------------------|----------------------------------|-------------------|-------------------------|
| `BASE32_ENC`       | Standard Base32 encode selector  | ❌ No effect      | ✔️ Yes                 |
| `BASE32_DEC`       | Standard Base32 decode selector  | ❌ No effect      | ✔️ Yes                 |
| `BASE32_ENC_NOPAD` | Encode without `=` padding       | ✔️ Yes            | ✔️ Yes                 |
| `BASE32_DEC_NOPAD` | Decode without requiring padding | ✔️ Yes            | ✔️ Yes                 |

> `BASE32_ENC` and `BASE32_DEC` exist only for **clarity and length calculations**, not for the algorithm itself.

---

## 🔢 Base58 Flags

| Flag         | Meaning         | Affects Encoding? | Used by Length Helpers? |
|--------------|-----------------|-------------------|-------------------------|
| `BASE58_ENC` | Encode selector | ❌ No effect     | ✔️ Yes                  |
| `BASE58_DEC` | Decode selector | ❌ No effect     | ✔️ Yes                  |

> Base58 has **one alphabet** and **no padding**, so flags are purely for wrappers and length calculations.

---

## 🧬 Base64 Flags (Standard + URL-safe)

| Flag               | Meaning                                    | Affects Encoding? | Used by Length Helpers? |
|--------------------|--------------------------------------------|-------------------|-------------------------|
| `BASE64_STD_ENC`   | Standard Base64 encode (A–Z a–z 0–9 + /)   | ✔️ Yes           | ✔️ Yes                  |
| `BASE64_STD_DEC`   | Standard Base64 decode                     | ✔️ Yes           | ✔️ Yes                  |
| `BASE64_URL_ENC`   | URL-safe alphabet encode (A–Z a–z 0–9 - _) | ✔️ Yes           | ✔️ Yes                  |
| `BASE64_URL_DEC`   | URL-safe alphabet decode                   | ✔️ Yes           | ✔️ Yes                  |
| `BASE64_NOPAD_ENC` | Encode without `=` padding                 | ✔️ Yes           | ✔️ Yes                  |
| `BASE64_NOPAD_DEC` | Decode without requiring padding           | ✔️ Yes           | ✔️ Yes                  |

> Base64 always consults these flags because it supports two alphabets and optional padding.

---

## 🧱 Base85 Flags (Standard, Extended, Z85)

| Flag               | Meaning                           | Affects Encoding? | Used by Length Helpers? |
|--------------------|-----------------------------------|-------------------|-------------------------|
| `BASE85_STD_ENC`   | Standard ASCII85 encode           | ✔️ Yes           | ✔️ Yes                  |
| `BASE85_STD_DEC`   | Standard ASCII85 decode           | ✔️ Yes           | ✔️ Yes                  |
| `BASE85_EXT_ENC`   | Extended Adobe ASCII85 encode     | ✔️ Yes           | ✔️ Yes                  |
| `BASE85_EXT_DEC`   | Extended Adobe ASCII85 decode     | ✔️ Yes           | ✔️ Yes                  |
| `BASE85_Z85_ENC`   | ZeroMQ Z85 encode                 | ✔️ Yes           | ✔️ Yes                  |
| `BASE85_Z85_DEC`   | ZeroMQ Z85 decode                 | ✔️ Yes           | ✔️ Yes                  |
| `BASE85_IGNORE_WS` | Ignore whitespace during decoding | ✔️ Yes           | ❌ No                   |

> `BASE85_IGNORE_WS` only affects decoding functions, allowing input with spaces, tabs, or newlines to be ignored.

---

**Note:** Wrappers like `BASE16_EncodeUpper()`, `BASE32_EncodeStdNoPad()`, or `BASE64_EncodeUrlNoPad()` automatically select the correct flags for convenience.  
Raw functions are only needed if you want **manual control over behavior** or for testing.

---

## Estimated Encoding Size Changes

When encoding binary data, different schemes increase the output size by different ratios. The table below summarizes the typical increase in size compared to the original input:

| Encoding Type     | Estimated Increase / Decrease in Size                |
|-------------------|------------------------------------------------------|
| **Base16 / Hex**  | +100% (2 output bytes per input byte)                |
| **Base32**        | +60% (8 output bytes per 5 input bytes)              |
| **Base58**        | ~+38% (approximate, varies with input)               |
| **Base64**        | +33% (4 output bytes per 3 input bytes)              |
| **Base85**        | +25% (5 output bytes per 4 input bytes)              |
| **Base85 / Z85**  | +25% (5/4 ratio, input length must be multiple of 4) |

> ⚠️ Note: These are theoretical maximum increases; actual encoded length may vary slightly due to padding or ignored whitespace in some variants.

## Sources / References

- [RFC 4648 – The Base16, Base32, and Base64 Data Encodings, October 2006](https://datatracker.ietf.org/doc/html/rfc4648)
- [RFC 3548 – Base16, Base32, and Base64 Data Encodings, July 2003](https://datatracker.ietf.org/doc/html/rfc3548)
- [Ascii85 – ASCII/Base85 Binary‑to‑Text Encoding (btoa/Adobe/PDF/PostScript), circa 1990s](https://en.wikipedia.org/wiki/Ascii85)
- [RFC 32 – The Z85 (ZeroMQ Base85) Data Encoding, March 2010](https://rfc.zeromq.org/spec/32/)

---

## License

This project is released under the **MIT License**. See '[LICENSE](LICENSE)' for full text.
