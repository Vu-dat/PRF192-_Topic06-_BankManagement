#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "transaction.h"

#define EPSILON 1e-9

static int tm_findAccountIndex(const AccountManager *am, const char *accNum) {
    int i;

    if (am == NULL || accNum == NULL || am->accounts == NULL) {
        return -1;
    }

    for (i = 0; i < am->count; i++) {
        if (strcmp(am->accounts[i].accountNumber, accNum) == 0) {
            return i;
        }
    }
    return -1;
}

int tm_deposit(AccountManager *am, const char *accNum, double amount) {
    int index;

    if (am == NULL || accNum == NULL) {
        return ERROR_INVALID_DATA;
    }
    if (accNum[0] == '\0') {
        return ERROR_INVALID_DATA;
    }
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) {
        return ERROR_INVALID_DATA;
    }
    if (amount <= 0.0 || isnan(amount) || isinf(amount)) {
        return ERROR_INVALID_DATA;
    }

    index = tm_findAccountIndex(am, accNum);
    if (index < 0) {
        return ERROR_NOT_FOUND;
    }

    am->accounts[index].balance += amount;
    return SUCCESS;
}

int tm_withdraw(AccountManager *am, const char *accNum, double amount) {
    int index;

    if (am == NULL || accNum == NULL) {
        return ERROR_INVALID_DATA;
    }
    if (accNum[0] == '\0') {
        return ERROR_INVALID_DATA;
    }
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) {
        return ERROR_INVALID_DATA;
    }
    if (amount <= 0.0 || isnan(amount) || isinf(amount)) {
        return ERROR_INVALID_DATA;
    }

    index = tm_findAccountIndex(am, accNum);
    if (index < 0) {
        return ERROR_NOT_FOUND;
    }

    if (am->accounts[index].balance + EPSILON < amount) {
        return ERROR_INSUFFICIENT_BALANCE;
    }

    am->accounts[index].balance -= amount;
    if (fabs(am->accounts[index].balance) < EPSILON) {
        am->accounts[index].balance = 0.0;
    }

    return SUCCESS;
}

int tm_transfer(AccountManager *am, const char *srcAccNum, const char *destAccNum, double amount) {
    int srcIndex, destIndex;

    if (am == NULL || srcAccNum == NULL || destAccNum == NULL) {
        return ERROR_INVALID_DATA;
    }
    if (srcAccNum[0] == '\0' || destAccNum[0] == '\0') {
        return ERROR_INVALID_DATA;
    }
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) {
        return ERROR_INVALID_DATA;
    }
    if (amount <= 0.0 || isnan(amount) || isinf(amount)) {
        return ERROR_INVALID_DATA;
    }
    if (strcmp(srcAccNum, destAccNum) == 0) {
        return ERROR_INVALID_DATA;
    }

    srcIndex = tm_findAccountIndex(am, srcAccNum);
    if (srcIndex < 0) {
        return ERROR_NOT_FOUND;
    }

    destIndex = tm_findAccountIndex(am, destAccNum);
    if (destIndex < 0) {
        return ERROR_NOT_FOUND;
    }

    if (am->accounts[srcIndex].balance + EPSILON < amount) {
        return ERROR_INSUFFICIENT_BALANCE;
    }

    am->accounts[srcIndex].balance -= amount;
    if (fabs(am->accounts[srcIndex].balance) < EPSILON) {
        am->accounts[srcIndex].balance = 0.0;
    }
    am->accounts[destIndex].balance += amount;

    return SUCCESS;
}

void tm_sortByBalanceOrder(AccountManager *am, int ascending) {
    int i, j;
    Account temp;

    if (am == NULL || am->accounts == NULL || am->count <= 1) {
        return;
    }

    for (i = 0; i < am->count - 1; i++) {
        for (j = 0; j < am->count - 1 - i; j++) {
            int shouldSwap = 0;
            if (ascending) {
                if (am->accounts[j].balance > am->accounts[j + 1].balance) {
                    shouldSwap = 1;
                }
            } else {
                if (am->accounts[j].balance < am->accounts[j + 1].balance) {
                    shouldSwap = 1;
                }
            }

            if (shouldSwap) {
                temp = am->accounts[j];
                am->accounts[j] = am->accounts[j + 1];
                am->accounts[j + 1] = temp;
            }
        }
    }
}

void tm_sortByBalance(AccountManager *am) {
    tm_sortByBalanceOrder(am, 0);
}
