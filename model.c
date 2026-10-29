#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "bank_system.h"

static int ensureAccountCapacity(AccountManager *manager)
{
    Account *newItems;
    size_t newCapacity;

    if (manager == NULL) {
        return 0;
    }
    if (manager->count < manager->capacity) {
        return 1;
    }
    if (manager->capacity >= MAX_ACCOUNTS) {
        return 0;
    }

    newCapacity = manager->capacity == 0 ? INITIAL_CAPACITY : manager->capacity * 2;
    if (newCapacity > MAX_ACCOUNTS) {
        newCapacity = MAX_ACCOUNTS;
    }

    newItems = realloc(manager->accounts, newCapacity * sizeof(Account));
    if (newItems == NULL) {
        return 0;
    }

    manager->accounts = newItems;
    manager->capacity = newCapacity;
    return 1;
}

int initializeAccountManager(AccountManager *manager)
{
    if (manager == NULL) {
        return 0;
    }
    manager->accounts = NULL;
    manager->count = 0;
    manager->capacity = 0;
    return 1;
}

void freeAccountManager(AccountManager *manager)
{
    if (manager == NULL) {
        return;
    }
    free(manager->accounts);
    manager->accounts = NULL;
    manager->count = 0;
    manager->capacity = 0;
}

int validateAccount(const Account *account)
{
    size_t i;

    if (account == NULL || account->accountNumber[0] == '\0' ||
        account->holderName[0] == '\0' || account->accountType[0] == '\0' ||
        account->openingDate[0] == '\0') {
        return 0;
    }
    if (!isfinite(account->balance) || account->balance < MIN_INITIAL_BALANCE) {
        return 0;
    }
    if (strncmp(account->accountNumber, "ACC", ACCOUNT_PREFIX_LENGTH) != 0 ||
        strlen(account->accountNumber) <= ACCOUNT_PREFIX_LENGTH) {
        return 0;
    }
    for (i = ACCOUNT_PREFIX_LENGTH; account->accountNumber[i] != '\0'; i++) {
        if (!isdigit((unsigned char)account->accountNumber[i])) {
            return 0;
        }
    }
    return 1;
}

int findAccountIndex(const AccountManager *manager, const char *accountNumber)
{
    size_t i;

    if (manager == NULL || accountNumber == NULL || accountNumber[0] == '\0') {
        return -1;
    }
    for (i = 0; i < manager->count; i++) {
        if (strcmp(manager->accounts[i].accountNumber, accountNumber) == 0) {
            return (int)i;
        }
    }
    return -1;
}

int addAccount(AccountManager *manager, const Account *account)
{
    if (manager == NULL || account == NULL || !validateAccount(account)) {
        return 0;
    }
    if (findAccountIndex(manager, account->accountNumber) >= 0) {
        return 0;
    }
    if (!ensureAccountCapacity(manager)) {
        return 0;
    }

    manager->accounts[manager->count] = *account;
    manager->count++;
    return 1;
}
