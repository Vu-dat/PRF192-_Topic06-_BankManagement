#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bank_system.h"

static void clearInputLine(void)
{
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
        /* Discard the rest of the current input line. */
    }
}

int readText(const char *prompt, char *buffer, size_t bufferSize)
{
    size_t length;

    if (prompt == NULL || buffer == NULL || bufferSize < 2) {
        return 0;
    }

    printf("%s", prompt);
    if (fgets(buffer, (int)bufferSize, stdin) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        clearInputLine();
        return 0;
    }

    if (buffer[0] == '\0') {
        return 0;
    }
    return 1;
}

int readMenuChoice(int *choice)
{
    char line[32];
    char *end;
    long value;

    if (choice == NULL) {
        return 0;
    }
    if (!readText("Choose an option: ", line, sizeof line)) {
        return 0;
    }

    errno = 0;
    value = strtol(line, &end, 10);
    if (errno != 0 || end == line || *end != '\0' ||
        value < MENU_MIN_CHOICE || value > MENU_MAX_CHOICE) {
        return 0;
    }

    *choice = (int)value;
    return 1;
}

int readAmount(const char *prompt, double *amount)
{
    char line[64];
    char *end;
    double value;

    if (amount == NULL || !readText(prompt, line, sizeof line)) {
        return 0;
    }

    errno = 0;
    value = strtod(line, &end);
    if (errno != 0 || end == line || *end != '\0' ||
        !isfinite(value) || value <= 0.0) {
        return 0;
    }

    *amount = value;
    return 1;
}

void displayMainMenu(void)
{
    printf("\n====== BANK ACCOUNT SYSTEM ======\n");
    printf("1. Create account\n");
    printf("2. Display accounts\n");
    printf("3. Deposit money\n");
    printf("4. Withdraw money\n");
    printf("5. Transfer money\n");
    printf("6. Display transaction history\n");
    printf("7. Save data now\n");
    printf("0. Exit\n");
}

void displayAccounts(const AccountManager *manager)
{
    size_t i;

    if (manager == NULL) {
        printf("Account data is unavailable.\n");
        return;
    }
    if (manager->count == 0) {
        printf("No accounts found.\n");
        return;
    }

    printf("\n%-12s %-22s %-12s %12s %-12s\n",
           "Account No.", "Holder", "Type", "Balance", "Open Date");
    for (i = 0; i < manager->count; i++) {
        const Account *account = &manager->accounts[i];
        printf("%-12s %-22s %-12s %12.2f %-12s\n",
               account->accountNumber, account->holderName,
               account->accountType, account->balance,
               account->openingDate);
    }
}

void displayTransactions(const TransactionManager *manager)
{
    size_t i;

    if (manager == NULL) {
        printf("Transaction data is unavailable.\n");
        return;
    }
    if (manager->count == 0) {
        printf("No transactions found.\n");
        return;
    }

    printf("\n%-12s %-12s %-12s %-12s %10s %-20s\n",
           "ID", "Source", "Destination", "Type", "Amount", "Timestamp");
    for (i = 0; i < manager->count; i++) {
        const Transaction *transaction = &manager->items[i];
        printf("%-12s %-12s %-12s %-12s %10.2f %-20s\n",
               transaction->transactionID, transaction->sourceAccNum,
               transaction->destAccNum, transaction->type,
               transaction->amount, transaction->timestamp);
    }
}

static void createAccountFromInput(AccountManager *accounts)
{
    Account account;

    if (!readText("Account number (ACCxxx): ", account.accountNumber,
                  sizeof account.accountNumber) ||
        !readText("Holder name: ", account.holderName, sizeof account.holderName) ||
        !readText("Account type (Savings/Checking): ", account.accountType,
                  sizeof account.accountType) ||
        !readText("Opening date (YYYY-MM-DD): ", account.openingDate,
                  sizeof account.openingDate)) {
        printf("Invalid or overly long input. Account was not created.\n");
        return;
    }
    account.balance = 0.0;

    if (addAccount(accounts, &account)) {
        printf("Account created successfully.\n");
    } else {
        printf("Account was not created. Check the data or duplicate account number.\n");
    }
}

static void processDeposit(AccountManager *accounts, TransactionManager *transactions)
{
    char accountNumber[MAX_ACCOUNT_NUMBER];
    char timestamp[MAX_TIMESTAMP];
    double amount;

    if (!readText("Account number: ", accountNumber, sizeof accountNumber) ||
        !readAmount("Amount to deposit: ", &amount) ||
        !readText("Timestamp (YYYY-MM-DD HH:MM): ", timestamp, sizeof timestamp)) {
        printf("Invalid input. Deposit cancelled.\n");
        return;
    }

    if (depositMoney(accounts, transactions, accountNumber, amount, timestamp)) {
        printf("Deposit successful.\n");
    } else {
        printf("Deposit failed. Check the account and amount.\n");
    }
}

static void processWithdrawal(AccountManager *accounts, TransactionManager *transactions)
{
    char accountNumber[MAX_ACCOUNT_NUMBER];
    char timestamp[MAX_TIMESTAMP];
    double amount;

    if (!readText("Account number: ", accountNumber, sizeof accountNumber) ||
        !readAmount("Amount to withdraw: ", &amount) ||
        !readText("Timestamp (YYYY-MM-DD HH:MM): ", timestamp, sizeof timestamp)) {
        printf("Invalid input. Withdrawal cancelled.\n");
        return;
    }

    if (withdrawMoney(accounts, transactions, accountNumber, amount, timestamp)) {
        printf("Withdrawal successful.\n");
    } else {
        printf("Withdrawal failed. Check the account, amount, or available balance.\n");
    }
}

static void processTransfer(AccountManager *accounts, TransactionManager *transactions)
{
    char sourceNumber[MAX_ACCOUNT_NUMBER];
    char destinationNumber[MAX_ACCOUNT_NUMBER];
    char timestamp[MAX_TIMESTAMP];
    double amount;

    if (!readText("Source account: ", sourceNumber, sizeof sourceNumber) ||
        !readText("Destination account: ", destinationNumber, sizeof destinationNumber) ||
        !readAmount("Amount to transfer: ", &amount) ||
        !readText("Timestamp (YYYY-MM-DD HH:MM): ", timestamp, sizeof timestamp)) {
        printf("Invalid input. Transfer cancelled.\n");
        return;
    }

    if (transferMoney(accounts, transactions, sourceNumber, destinationNumber,
                      amount, timestamp)) {
        printf("Transfer successful.\n");
    } else {
        printf("Transfer failed. Check accounts, amount, and source balance.\n");
    }
}

void runUserInterface(AccountManager *accounts, TransactionManager *transactions)
{
    int choice = -1;
    int inputValid;

    if (accounts == NULL || transactions == NULL) {
        printf("Cannot start menu: system data is unavailable.\n");
        return;
    }

    while (choice != MENU_EXIT_CHOICE) {
        displayMainMenu();
        inputValid = readMenuChoice(&choice);
        if (!inputValid) {
            printf("Invalid choice. Enter a number from %d to %d.\n",
                   MENU_MIN_CHOICE, MENU_MAX_CHOICE);
            choice = -1;
        } else {
            switch (choice) {
                case 1:
                    createAccountFromInput(accounts);
                    break;
                case 2:
                    displayAccounts(accounts);
                    break;
                case 3:
                    processDeposit(accounts, transactions);
                    break;
                case 4:
                    processWithdrawal(accounts, transactions);
                    break;
                case 5:
                    processTransfer(accounts, transactions);
                    break;
                case 6:
                    displayTransactions(transactions);
                    break;
                case 7:
                    if (saveAccounts(accounts, ACCOUNT_FILE) &&
                        saveTransactions(transactions, TRANSACTION_FILE)) {
                        printf("Data saved successfully.\n");
                    } else {
                        printf("Could not save all data.\n");
                    }
                    break;
                case MENU_EXIT_CHOICE:
                    printf("Exiting. Data will be saved.\n");
                    break;
                default:
                    printf("Invalid choice.\n");
                    break;
            }
        }
    }
}
