#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bank_system.h"

static int ensureTransactionCapacity(TransactionManager *manager)
{
    Transaction *newItems;
    size_t newCapacity;

    if (manager == NULL) {
        return 0;
    }
    if (manager->count < manager->capacity) {
        return 1;
    }
    if (manager->capacity >= MAX_TRANSACTIONS) {
        return 0;
    }

    newCapacity = manager->capacity == 0 ? INITIAL_CAPACITY : manager->capacity * 2;
    if (newCapacity > MAX_TRANSACTIONS) {
        newCapacity = MAX_TRANSACTIONS;
    }

    newItems = realloc(manager->items, newCapacity * sizeof(Transaction));
    if (newItems == NULL) {
        return 0;
    }

    manager->items = newItems;
    manager->capacity = newCapacity;
    return 1;
}

int initializeTransactionManager(TransactionManager *manager)
{
    if (manager == NULL) {
        return 0;
    }
    manager->items = NULL;
    manager->count = 0;
    manager->capacity = 0;
    return 1;
}

void freeTransactionManager(TransactionManager *manager)
{
    if (manager == NULL) {
        return;
    }
    free(manager->items);
    manager->items = NULL;
    manager->count = 0;
    manager->capacity = 0;
}

int createTransactionRecord(Transaction *transaction, const char *transactionID,
                            const char *sourceNumber, const char *destinationNumber,
                            const char *type, double amount, const char *timestamp)
{
    if (transaction == NULL || transactionID == NULL || sourceNumber == NULL ||
        destinationNumber == NULL || type == NULL || timestamp == NULL ||
        transactionID[0] == '\0' || type[0] == '\0' ||
        timestamp[0] == '\0' || !isfinite(amount) || amount <= 0.0) {
        return 0;
    }
    if (snprintf(transaction->transactionID, sizeof transaction->transactionID,
                 "%s", transactionID) >= (int)sizeof transaction->transactionID ||
        snprintf(transaction->sourceAccNum, sizeof transaction->sourceAccNum,
                 "%s", sourceNumber) >= (int)sizeof transaction->sourceAccNum ||
        snprintf(transaction->destAccNum, sizeof transaction->destAccNum,
                 "%s", destinationNumber) >= (int)sizeof transaction->destAccNum ||
        snprintf(transaction->type, sizeof transaction->type,
                 "%s", type) >= (int)sizeof transaction->type ||
        snprintf(transaction->timestamp, sizeof transaction->timestamp,
                 "%s", timestamp) >= (int)sizeof transaction->timestamp) {
        return 0;
    }
    transaction->amount = amount;
    return 1;
}

int addTransaction(TransactionManager *manager, const Transaction *transaction)
{
    if (manager == NULL || transaction == NULL ||
        transaction->transactionID[0] == '\0' ||
        !isfinite(transaction->amount) || transaction->amount <= 0.0) {
        return 0;
    }
    if (!ensureTransactionCapacity(manager)) {
        return 0;
    }
    manager->items[manager->count] = *transaction;
    manager->count++;
    return 1;
}

static int prepareTransaction(Transaction *record, const TransactionManager *transactions,
                              const char *sourceNumber, const char *destinationNumber,
                              const char *type, double amount, const char *timestamp)
{
    char transactionID[MAX_TRANSACTION_ID];

    if (transactions == NULL || sourceNumber == NULL || destinationNumber == NULL ||
        type == NULL || timestamp == NULL) {
        return 0;
    }
    if (snprintf(transactionID, sizeof transactionID, "TXN%05lu",
                 (unsigned long)(transactions->count + 1)) >= (int)sizeof transactionID) {
        return 0;
    }
    return createTransactionRecord(record, transactionID, sourceNumber,
                                   destinationNumber, type, amount, timestamp);
}

int depositMoney(AccountManager *accounts, TransactionManager *transactions,
                 const char *accountNumber, double amount, const char *timestamp)
{
    int index;
    Transaction record;

    if (accounts == NULL || transactions == NULL || accountNumber == NULL ||
        timestamp == NULL || !isfinite(amount) || amount <= 0.0) {
        return 0;
    }
    index = findAccountIndex(accounts, accountNumber);
    if (index < 0) {
        return 0;
    }
    if (!prepareTransaction(&record, transactions, accountNumber, "-", "Deposit",
                            amount, timestamp)) {
        return 0;
    }
    if (!addTransaction(transactions, &record)) {
        return 0;
    }
    accounts->accounts[index].balance += amount;
    return 1;
}

int withdrawMoney(AccountManager *accounts, TransactionManager *transactions,
                  const char *accountNumber, double amount, const char *timestamp)
{
    int index;
    Transaction record;

    if (accounts == NULL || transactions == NULL || accountNumber == NULL ||
        timestamp == NULL || !isfinite(amount) || amount <= 0.0) {
        return 0;
    }
    index = findAccountIndex(accounts, accountNumber);
    if (index < 0 || accounts->accounts[index].balance + AMOUNT_EPSILON < amount) {
        return 0;
    }
    if (!prepareTransaction(&record, transactions, accountNumber, "-", "Withdrawal",
                            amount, timestamp)) {
        return 0;
    }
    if (!addTransaction(transactions, &record)) {
        return 0;
    }
    accounts->accounts[index].balance -= amount;
    return 1;
}

int transferMoney(AccountManager *accounts, TransactionManager *transactions,
                  const char *sourceNumber, const char *destinationNumber,
                  double amount, const char *timestamp)
{
    int sourceIndex;
    int destinationIndex;
    Transaction record;

    if (accounts == NULL || transactions == NULL || sourceNumber == NULL ||
        destinationNumber == NULL || timestamp == NULL || !isfinite(amount) || amount <= 0.0 ||
        strcmp(sourceNumber, destinationNumber) == 0) {
        return 0;
    }
    sourceIndex = findAccountIndex(accounts, sourceNumber);
    destinationIndex = findAccountIndex(accounts, destinationNumber);
    if (sourceIndex < 0 || destinationIndex < 0 ||
        accounts->accounts[sourceIndex].balance + AMOUNT_EPSILON < amount) {
        return 0;
    }
    if (!prepareTransaction(&record, transactions, sourceNumber, destinationNumber,
                            "Transfer", amount, timestamp)) {
        return 0;
    }
    if (!addTransaction(transactions, &record)) {
        return 0;
    }

    accounts->accounts[sourceIndex].balance -= amount;
    accounts->accounts[destinationIndex].balance += amount;
    return 1;
}
