#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "account.h"

static int am_findAccountIndex(const AccountManager *am, const char *accNum) {
    int i;

    if (am == NULL || accNum == NULL) return -1;
    for (i = 0; i < am->count; i++) {
        if (strcmp(am->accounts[i].accountNumber, accNum) == 0) return i;
    }
    return -1;
}

int am_openAccount(AccountManager *am, const Account *newAcc) {
    Account *temp;
    time_t now;
    struct tm *local;

    if (am == NULL || newAcc == NULL) return ERROR_INVALID_DATA;
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) return ERROR_INVALID_DATA;
    if (newAcc->accountNumber[0] == '\0' || newAcc->holderName[0] == '\0') return ERROR_INVALID_DATA;
    if (newAcc->balance < 0) return ERROR_INVALID_DATA;
    if (am_findAccountIndex(am, newAcc->accountNumber) >= 0) return ERROR_DUPLICATE_ACCOUNT;

    temp = (Account *)realloc(am->accounts, (size_t)(am->count + 1) * sizeof(Account));
    if (temp == NULL) return ERROR_MEMORY_FULL;

    am->accounts = temp;
    am->accounts[am->count] = *newAcc;

    now = time(NULL);
    local = localtime(&now);
    if (local != NULL) {
        strftime(am->accounts[am->count].openingDate,
                 sizeof(am->accounts[am->count].openingDate),
                 "%Y-%m-%d", local);
    } else {
        am->accounts[am->count].openingDate[0] = '\0';
    }

    am->count++;
    return SUCCESS;
}

int am_updateHolderInfo(AccountManager *am, const char *accNum, const char *newName) {
    int index;

    if (am == NULL || accNum == NULL || newName == NULL) return ERROR_INVALID_DATA;
    if (accNum[0] == '\0' || newName[0] == '\0') return ERROR_INVALID_DATA;
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) return ERROR_INVALID_DATA;
    if (strlen(newName) >= sizeof(am->accounts[0].holderName)) return ERROR_INVALID_DATA;

    index = am_findAccountIndex(am, accNum);
    if (index < 0) return ERROR_NOT_FOUND;

    strcpy(am->accounts[index].holderName, newName);
    return SUCCESS;
}

int am_searchAccount(const AccountManager *am, const char *keyword, int *outIndexes, int maxOut) {
    int i;
    int found = 0;

    if (am == NULL || keyword == NULL || outIndexes == NULL) return 0;
    if (maxOut <= 0 || keyword[0] == '\0') return 0;
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) return 0;

    for (i = 0; i < am->count && found < maxOut; i++) {
        if (strstr(am->accounts[i].accountNumber, keyword) != NULL ||
            strstr(am->accounts[i].holderName, keyword) != NULL) {
            outIndexes[found] = i;
            found++;
        }
    }
    return found;
}

int am_deleteAccount(AccountManager *am, const char *accNum) {
    int index;
    int i;
    Account *temp;

    if (am == NULL || accNum == NULL || accNum[0] == '\0') {
        return ERROR_INVALID_DATA;
    }
    if (am->count <= 0 || am->accounts == NULL) {
        return ERROR_NOT_FOUND;
    }

    index = am_findAccountIndex(am, accNum);
    if (index < 0) {
        return ERROR_NOT_FOUND;
    }

    for (i = index; i < am->count - 1; i++) {
        am->accounts[i] = am->accounts[i + 1];
    }

    am->count--;
    if (am->count == 0) {
        free(am->accounts);
        am->accounts = NULL;
        return SUCCESS;
    }

    temp = (Account *)realloc(am->accounts, (size_t)am->count * sizeof(Account));
    if (temp != NULL) {
        am->accounts = temp;
    }
    return SUCCESS;
}

void am_freeAccounts(AccountManager *am) {
    if (am == NULL) return;
    free(am->accounts);
    am->accounts = NULL;
    am->count = 0;
}
