#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

#define UTILS_ACCNUM_MAX_LEN 19
#define UTILS_ACCNUM_MIN_LEN 4

void utils_trimWhitespace(char *str) {
    char *start;
    char *end;
    size_t len;

    if (str == NULL) return;

    start = str;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    len = strlen(start);
    end = start + len;
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    *end = '\0';

    if (start != str) {
        memmove(str, start, (size_t)(end - start) + 1);
    }
}

void utils_formatCurrency(double amount, char *outStr, size_t maxLen) {
    char raw[64];
    const char *src;
    const char *signStr;
    const char *dotPos;
    size_t offset;
    size_t intLen;
    size_t sepCount;
    size_t totalLen;
    size_t srcIdx, dstIdx;
    int written;
    int digits;
    int leadDigits;

    if (outStr == NULL || maxLen == 0) return;
    outStr[0] = '\0';

    written = snprintf(raw, sizeof(raw), "%.2f", amount);
    if (written < 0 || (size_t)written >= sizeof(raw)) return;

    if (raw[0] == '-' || raw[0] == '+') {
        signStr = (raw[0] == '-') ? "-" : "+";
        src = raw + 1;
    } else {
        signStr = "";
        src = raw;
    }

    dotPos = strchr(src, '.');
    intLen = (dotPos != NULL) ? (size_t)(dotPos - src) : strlen(src);
    sepCount = (intLen > 0) ? (intLen - 1) / 3 : 0;

    offset = strlen(signStr);
    totalLen = offset + strlen(src) + sepCount;

    if (totalLen + 1 > maxLen) {
        snprintf(outStr, maxLen, "%.*f", 2, amount);
        return;
    }

    memcpy(outStr, signStr, offset);
    dstIdx = offset;
    srcIdx = 0;
    leadDigits = (intLen > 0) ? (int)(intLen % 3) : 0;
    if (leadDigits == 0 && intLen > 0) leadDigits = 3;
    while (srcIdx < (size_t)leadDigits) {
        outStr[dstIdx++] = src[srcIdx++];
    }
    while (srcIdx < intLen) {
        outStr[dstIdx++] = (char)UTILS_THOUSAND_SEP;
        digits = 0;
        while (srcIdx < intLen && digits < 3) {
            outStr[dstIdx++] = src[srcIdx++];
            digits++;
        }
    }
    while (src[srcIdx] != '\0') {
        outStr[dstIdx++] = src[srcIdx++];
    }
    outStr[dstIdx] = '\0';
}

int utils_isValidAccountNumber(const char *accNum) {
    size_t len;
    size_t i;

    if (accNum == NULL) return 0;

    len = strlen(accNum);
    if (len < UTILS_ACCNUM_MIN_LEN || len > UTILS_ACCNUM_MAX_LEN) return 0;

    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)accNum[i])) return 0;
    }
    return 1;
}

int utils_isValidName(const char *name) {
    size_t len;
    size_t i;
    unsigned char c;
    int hasVisibleChar = 0;

    if (name == NULL) return 0;

    len = strlen(name);
    if (len == 0) return 0;

    for (i = 0; i < len; i++) {
        c = (unsigned char)name[i];

        if (c >= 0x80) {
            hasVisibleChar = 1;
            continue;
        }
        if (isalpha(c)) {
            hasVisibleChar = 1;
            continue;
        }
        if (c == ' ') {
            if (!hasVisibleChar) return 0;
            if (name[i + 1] == '\0') return 0;
            continue;
        }
        if (c == '.' || c == '-' || c == '\'') {
            hasVisibleChar = 1;
            continue;
        }
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (name[i] != ' ') return 1;
    }
    return 0;
}

void utils_clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void utils_pauseScreen(void) {
    int ch;

    printf("Press Enter to continue...");
    fflush(stdout);
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}
