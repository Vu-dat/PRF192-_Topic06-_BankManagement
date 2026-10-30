#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define UTILS_THOUSAND_SEP ','

void utils_trimWhitespace(char *str);
void utils_formatCurrency(double amount, char *outStr, size_t maxLen);
int utils_isValidAccountNumber(const char *accNum);
int utils_isValidName(const char *name);
void utils_clearScreen(void);
void utils_pauseScreen(void);

#endif
