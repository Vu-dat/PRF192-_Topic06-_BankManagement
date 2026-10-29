#ifndef BANK_SYSTEM_H
#define BANK_SYSTEM_H

#include <stddef.h>

#define ACCOUNT_FILE "accounts_data.txt"
#define TRANSACTION_FILE "transactions_data.txt"
#define MAX_ACCOUNT_NUMBER 20
#define MAX_HOLDER_NAME 100
#define MAX_ACCOUNT_TYPE 20
#define MAX_DATE_TEXT 20
#define MAX_TRANSACTION_ID 30
#define MAX_TIMESTAMP 30
#define INITIAL_CAPACITY 4
#define ACCOUNT_PREFIX_LENGTH 3
#define MENU_MIN_CHOICE 0
#define MENU_MAX_CHOICE 7
#define MENU_EXIT_CHOICE 0
#define MAX_ACCOUNTS 1000
#define MAX_TRANSACTIONS 5000
#define MIN_INITIAL_BALANCE 0.0
#define AMOUNT_EPSILON 0.000001

typedef struct {
    char accountNumber[MAX_ACCOUNT_NUMBER];
    char holderName[MAX_HOLDER_NAME];
    char accountType[MAX_ACCOUNT_TYPE];
    double balance;
    char openingDate[MAX_DATE_TEXT];
} Account;

typedef struct {
    char transactionID[MAX_TRANSACTION_ID];
    char sourceAccNum[MAX_ACCOUNT_NUMBER];
    char destAccNum[MAX_ACCOUNT_NUMBER];
    char type[MAX_ACCOUNT_TYPE];
    double amount;
    char timestamp[MAX_TIMESTAMP];
} Transaction;

typedef struct {
    Account *accounts;
    size_t count;
    size_t capacity;
} AccountManager;

typedef struct {
    Transaction *items;
    size_t count;
    size_t capacity;
} TransactionManager;

/* Account manager and validation */
int initializeAccountManager(AccountManager *manager);
void freeAccountManager(AccountManager *manager);
int findAccountIndex(const AccountManager *manager, const char *accountNumber);
int addAccount(AccountManager *manager, const Account *account);
int validateAccount(const Account *account);

/* Transaction manager and business operations */
int initializeTransactionManager(TransactionManager *manager);
void freeTransactionManager(TransactionManager *manager);
int addTransaction(TransactionManager *manager, const Transaction *transaction);
int depositMoney(AccountManager *accounts, TransactionManager *transactions,
                 const char *accountNumber, double amount, const char *timestamp);
int withdrawMoney(AccountManager *accounts, TransactionManager *transactions,
                  const char *accountNumber, double amount, const char *timestamp);
int transferMoney(AccountManager *accounts, TransactionManager *transactions,
                  const char *sourceNumber, const char *destinationNumber,
                  double amount, const char *timestamp);
int createTransactionRecord(Transaction *transaction, const char *transactionID,
                            const char *sourceNumber, const char *destinationNumber,
                            const char *type, double amount, const char *timestamp);

/* File I/O */
int saveAccounts(const AccountManager *manager, const char *filename);
int loadAccounts(AccountManager *manager, const char *filename);
int saveTransactions(const TransactionManager *manager, const char *filename);
int loadTransactions(TransactionManager *manager, const char *filename);

/* Console UI: user input/output only; business operations are in other functions */
void displayMainMenu(void);
int readMenuChoice(int *choice);
int readText(const char *prompt, char *buffer, size_t bufferSize);
int readAmount(const char *prompt, double *amount);
void displayAccounts(const AccountManager *manager);
void displayTransactions(const TransactionManager *manager);
void runUserInterface(AccountManager *accounts, TransactionManager *transactions);

/* Basic test runner */
int runTests(void);

#endif
