#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bank_system.h"

static int hasUnsafeText(const char *text)
{
    return text == NULL || strchr(text, ',') != NULL ||
           strchr(text, '\n') != NULL || strchr(text, '\r') != NULL;
}

int saveAccounts(const AccountManager *manager, const char *filename)
{
    FILE *file;
    size_t i;
    int success = 1;

    if (manager == NULL || filename == NULL || filename[0] == '\0' ||
        (manager->count > 0 && manager->accounts == NULL)) {
        return 0;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        return 0;
    }

    for (i = 0; i < manager->count; i++) {
        const Account *account = &manager->accounts[i];

        if (hasUnsafeText(account->accountNumber) ||
            hasUnsafeText(account->holderName) ||
            hasUnsafeText(account->accountType) ||
            hasUnsafeText(account->openingDate) ||
            fprintf(file, "%s,%s,%s,%.2f,%s\n",
                    account->accountNumber, account->holderName,
                    account->accountType, account->balance,
                    account->openingDate) < 0) {
            success = 0;
            break;
        }
    }

    if (fclose(file) != 0) {
        success = 0;
    }
    return success;
}

int loadAccounts(AccountManager *manager, const char *filename)
{
    FILE *file;
    AccountManager loaded;
    char line[256];
    int success = 1;

    if (manager == NULL || filename == NULL || filename[0] == '\0') {
        return 0;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }
    if (!initializeAccountManager(&loaded)) {
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof line, file) != NULL) {
        Account account;
        if (sscanf(line, "%19[^,],%99[^,],%19[^,],%lf,%19[^\r\n]",
                   account.accountNumber, account.holderName,
                   account.accountType, &account.balance,
                   account.openingDate) != 5 ||
            !validateAccount(&account) ||
            !addAccount(&loaded, &account)) {
            success = 0;
            break;
        }
    }

    if (ferror(file)) {
        success = 0;
    }
    if (fclose(file) != 0) {
        success = 0;
    }

    if (!success) {
        freeAccountManager(&loaded);
        return 0;
    }

    freeAccountManager(manager);
    *manager = loaded;
    return 1;
}

int saveTransactions(const TransactionManager *manager, const char *filename)
{
    FILE *file;
    size_t i;
    int success = 1;

    if (manager == NULL || filename == NULL || filename[0] == '\0' ||
        (manager->count > 0 && manager->items == NULL)) {
        return 0;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        return 0;
    }

    for (i = 0; i < manager->count; i++) {
        const Transaction *transaction = &manager->items[i];

        if (hasUnsafeText(transaction->transactionID) ||
            hasUnsafeText(transaction->sourceAccNum) ||
            hasUnsafeText(transaction->destAccNum) ||
            hasUnsafeText(transaction->type) ||
            hasUnsafeText(transaction->timestamp) ||
            fprintf(file, "%s,%s,%s,%s,%.2f,%s\n",
                    transaction->transactionID, transaction->sourceAccNum,
                    transaction->destAccNum, transaction->type,
                    transaction->amount, transaction->timestamp) < 0) {
            success = 0;
            break;
        }
    }

    if (fclose(file) != 0) {
        success = 0;
    }
    return success;
}

int loadTransactions(TransactionManager *manager, const char *filename)
{
    FILE *file;
    TransactionManager loaded;
    char line[256];
    int success = 1;

    if (manager == NULL || filename == NULL || filename[0] == '\0') {
        return 0;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }
    if (!initializeTransactionManager(&loaded)) {
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof line, file) != NULL) {
        Transaction transaction;
        if (sscanf(line, "%29[^,],%19[^,],%19[^,],%19[^,],%lf,%29[^\r\n]",
                   transaction.transactionID, transaction.sourceAccNum,
                   transaction.destAccNum, transaction.type,
                   &transaction.amount, transaction.timestamp) != 6 ||
            transaction.transactionID[0] == '\0' ||
            !isfinite(transaction.amount) || transaction.amount <= 0.0 ||
            !addTransaction(&loaded, &transaction)) {
            success = 0;
            break;
        }
    }

    if (ferror(file)) {
        success = 0;
    }
    if (fclose(file) != 0) {
        success = 0;
    }

    if (!success) {
        freeTransactionManager(&loaded);
        return 0;
    }

    freeTransactionManager(manager);
    *manager = loaded;
    return 1;
}
