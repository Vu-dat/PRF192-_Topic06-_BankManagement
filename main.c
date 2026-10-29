#include <stdio.h>
#include "bank_system.h"

/* All project function prototypes are declared in bank_system.h before main. */

int main(void)
{
    AccountManager accounts;
    TransactionManager transactions;
    int accountsReady;
    int transactionsReady;

    accountsReady = initializeAccountManager(&accounts);
    transactionsReady = initializeTransactionManager(&transactions);

    if (!accountsReady || !transactionsReady) {
        printf("Error: could not initialize system memory.\n");
        if (accountsReady) {
            freeAccountManager(&accounts);
        }
        if (transactionsReady) {
            freeTransactionManager(&transactions);
        }
        return 1;
    }

    if (!loadAccounts(&accounts, ACCOUNT_FILE)) {
        printf("Warning: account data could not be loaded. Starting with current in-memory data.\n");
    }
    if (!loadTransactions(&transactions, TRANSACTION_FILE)) {
        printf("Warning: transaction history could not be loaded.\n");
    }

    runUserInterface(&accounts, &transactions);

    if (!saveAccounts(&accounts, ACCOUNT_FILE)) {
        printf("Error: failed to save account data.\n");
    }
    if (!saveTransactions(&transactions, TRANSACTION_FILE)) {
        printf("Error: failed to save transaction history.\n");
    }

    freeAccountManager(&accounts);
    freeTransactionManager(&transactions);
    return 0;
}
