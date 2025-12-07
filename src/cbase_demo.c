#include "tiny_cbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_BUF 2048

void print_usage(const char *prog) {
    printf("Usage: %s <enc|dec> <base_flag> <input>\n", prog);
    printf("Base flags:\n");
    printf("  base16_upper, base16_lower\n");
    printf("  base32_std, base32_std_nopad\n");
    printf("  base58\n");
    printf("  base64_std, base64_url, base64_url_nopad\n");
    printf("  base85_std, base85_ext, base85_z85\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *base_flag = argv[2];
    const char *input_str = argv[3];

    uint8_t input_buf[MAX_BUF];
    size_t input_len = 0;

    // Detect if input is hex (for raw bytes) or string
    bool is_hex = false;
    if (strcmp(base_flag, "base16_upper") == 0 || strcmp(base_flag, "base16_lower") == 0 ||
        strcmp(base_flag, "base32_std") == 0 || strcmp(base_flag, "base32_std_nopad") == 0 ||
        strcmp(base_flag, "base58") == 0 || strcmp(base_flag, "base64_std") == 0 ||
        strcmp(base_flag, "base64_url") == 0 || strcmp(base_flag, "base64_url_nopad") == 0 ||
        strcmp(base_flag, "base85_std") == 0 || strcmp(base_flag, "base85_ext") == 0 ||
        strcmp(base_flag, "base85_z85") == 0) {
        is_hex = false; // treat input as string for raw encoding
    }

    if (!is_hex) {
        input_len = strlen(input_str);
        if (input_len > MAX_BUF) {
            fprintf(stderr, "Error: input too long (max %d bytes)\n", MAX_BUF);
            return 1;
        }
        memcpy(input_buf, input_str, input_len);
    }

    char encoded[MAX_BUF] = {0};
    uint8_t decoded[MAX_BUF] = {0};
    size_t enc_len = sizeof(encoded);
    size_t dec_len = sizeof(decoded);
    bool ok;

    // Encode or decode
    if (strcmp(mode, "enc") == 0) {
        // Z85 must be multiple of 4
        if (strcmp(base_flag, "base85_z85") == 0 && (input_len % 4 != 0)) {
            fprintf(stderr, "Error: Z85 input length must be multiple of 4\n");
            return 1;
        }

        if (strcmp(base_flag, "base16_upper") == 0) {
            ok = BASE16_EncodeUpper(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base16_lower") == 0) {
            ok = BASE16_EncodeLower(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base32_std") == 0) {
            ok = BASE32_EncodeStd(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base32_std_nopad") == 0) {
            ok = BASE32_EncodeStdNoPad(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base58") == 0) {
            ok = BASE58_Encode(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base64_std") == 0) {
            ok = BASE64_EncodeStd(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base64_url") == 0) {
            ok = BASE64_EncodeUrl(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base64_url_nopad") == 0) {
            ok = BASE64_EncodeUrlNoPad(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base85_std") == 0) {
            ok = BASE85_EncodeStd(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base85_ext") == 0) {
            ok = BASE85_EncodeExt(input_buf, input_len, encoded, &enc_len);
        } else if (strcmp(base_flag, "base85_z85") == 0) {
            ok = BASE85_EncodeZ85(input_buf, input_len, encoded, &enc_len);
        } else {
            fprintf(stderr, "Unknown base flag: %s\n", base_flag);
            return 1;
        }

        if (!ok) {
            fprintf(stderr, "Encoding failed\n");
            return 1;
        }

        printf("Encoded: %s\n", encoded);

    } else if (strcmp(mode, "dec") == 0) {
        enc_len = strlen(input_str);
        memcpy(encoded, input_str, enc_len);

        if (strcmp(base_flag, "base16_upper") == 0 || strcmp(base_flag, "base16_lower") == 0) {
            ok = BASE16_Decode(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base32_std") == 0) {
            ok = BASE32_DecodeStd(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base32_std_nopad") == 0) {
            ok = BASE32_DecodeStdNoPad(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base58") == 0) {
            ok = BASE58_Decode(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base64_std") == 0) {
            ok = BASE64_DecodeStd(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base64_url") == 0) {
            ok = BASE64_DecodeUrl(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base64_url_nopad") == 0) {
            ok = BASE64_DecodeUrlNoPad(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base85_std") == 0) {
            ok = BASE85_DecodeStd(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base85_ext") == 0) {
            ok = BASE85_DecodeExt(encoded, enc_len, decoded, &dec_len);
        } else if (strcmp(base_flag, "base85_z85") == 0) {
            if (enc_len % 5 != 0) {
                fprintf(stderr, "Error: Z85 decode length must be multiple of 5\n");
                return 1;
            }
            ok = BASE85_DecodeZ85(encoded, enc_len, decoded, &dec_len);
        } else {
            fprintf(stderr, "Unknown base flag: %s\n", base_flag);
            return 1;
        }

        if (!ok) {
            fprintf(stderr, "Decoding failed\n");
            return 1;
        }

        printf("Decoded (%zu bytes): ", dec_len);
        for (size_t i = 0; i < dec_len; i++) {
            putchar(decoded[i]);
        }
        printf("\n");

    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}