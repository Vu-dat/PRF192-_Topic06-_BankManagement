#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "account.h"
#include "transaction.h"
#include "file_io.h"

#define DEFAULT_ACCOUNT_FILE "data/accounts.txt"
#define DEFAULT_TRANSACTION_FILE "data/transactions.txt"
#define MAX_SEARCH_RESULTS 50

#define MENU_NONE           (-1)
#define MENU_EXIT           0
#define MENU_OPEN_ACCOUNT   1
#define MENU_UPDATE_HOLDER  2
#define MENU_SEARCH         3
#define MENU_DEPOSIT        4
#define MENU_WITHDRAW       5
#define MENU_TRANSFER       6
#define MENU_SORT           7
#define MENU_DISPLAY_ALL    8
#define MENU_SAVE           9
#define MENU_LOAD           10

void ui_displayMainMenu(void);
int  ui_readText(const char *prompt, char *buf, size_t size);
int  ui_readDouble(const char *prompt, double *outVal);
int  ui_readAccount(Account *acc);
void ui_printResult(int code);
void ui_printAccount(const Account *acc);
void ui_printSearchResults(const AccountManager *am, const int *indexes, int found);
void ui_displayAllAccounts(const AccountManager *am);
static void recordAuditLog(const char *srcAcc, const char *destAcc, const char *type, double amount);

static void recordAuditLog(const char *srcAcc, const char *destAcc, const char *type, double amount) {
    Transaction txn;
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    memset(&txn, 0, sizeof(txn));
    snprintf(txn.transactionID, sizeof(txn.transactionID), "TXN%lu", (unsigned long)now);
    strncpy(txn.sourceAcc, srcAcc ? srcAcc : "-", sizeof(txn.sourceAcc) - 1);
    strncpy(txn.destAcc, destAcc ? destAcc : "-", sizeof(txn.destAcc) - 1);
    strncpy(txn.type, type ? type : "Unknown", sizeof(txn.type) - 1);
    txn.amount = amount;
    if (local != NULL) {
        strftime(txn.timestamp, sizeof(txn.timestamp), "%Y-%m-%d %H:%M:%S", local);
    } else {
        strncpy(txn.timestamp, "N/A", sizeof(txn.timestamp) - 1);
    }
    file_logTransaction(DEFAULT_TRANSACTION_FILE, &txn);
}

int main(void) {
    AccountManager am;
    int choice = MENU_NONE;
    int ch;
    int loadStatus;

    am.accounts = NULL;
    am.count = 0;

    loadStatus = file_loadAccounts(&am, DEFAULT_ACCOUNT_FILE);
    if (loadStatus == SUCCESS && am.count > 0) {
        printf("[System] Successfully loaded %d account(s) from %s.\n",
               am.count, DEFAULT_ACCOUNT_FILE);
    }

    do {
        ui_displayMainMenu();
        if (scanf("%d", &choice) != 1) {
            if (feof(stdin)) {
                printf("\nInput stream closed. Exiting system.\n");
                choice = MENU_EXIT;
                continue;
            }
            printf("Invalid input! Please enter a number.\n");
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }
        while ((ch = getchar()) != '\n' && ch != EOF) { }

        switch (choice) {
            case MENU_OPEN_ACCOUNT: {
                Account newAcc = {0};
                int code;

                if (ui_readAccount(&newAcc) != SUCCESS) {
                    printf("Error: Invalid account information!\n");
                    break;
                }
                code = am_openAccount(&am, &newAcc);
                ui_printResult(code);
                if (code == SUCCESS) {
                    printf("Opened on: %s\n", am.accounts[am.count - 1].openingDate);
                }
                break;
            }
            case MENU_UPDATE_HOLDER: {
                char accNum[20] = {0};
                char newName[50] = {0};
                int code;

                if (ui_readText("Enter Account Number to update: ", accNum, sizeof(accNum)) != SUCCESS ||
                    ui_readText("Enter new holder name: ", newName, sizeof(newName)) != SUCCESS) {
                    printf("Error: Invalid input!\n");
                    break;
                }
                code = am_updateHolderInfo(&am, accNum, newName);
                ui_printResult(code);
                break;
            }
            case MENU_SEARCH: {
                char keyword[50] = {0};
                int indexes[MAX_SEARCH_RESULTS];
                int found;

                if (ui_readText("Enter keyword (Account Number or Name): ", keyword, sizeof(keyword)) != SUCCESS) {
                    printf("Error: Invalid input!\n");
                    break;
                }
                found = am_searchAccount(&am, keyword, indexes, MAX_SEARCH_RESULTS);
                ui_printSearchResults(&am, indexes, found);
                if (found == MAX_SEARCH_RESULTS) {
                    printf("Note: only the first %d matches are shown.\n", found);
                }
                break;
            }
            case MENU_DEPOSIT: {
                char accNum[20] = {0};
                double amount = 0.0;
                int code;

                if (ui_readText("Enter Account Number to deposit: ", accNum, sizeof(accNum)) != SUCCESS ||
                    ui_readDouble("Enter deposit amount: ", &amount) != SUCCESS) {
                    printf("Error: Invalid input!\n");
                    break;
                }
                code = tm_deposit(&am, accNum, amount);
                ui_printResult(code);
                if (code == SUCCESS) {
                    recordAuditLog(accNum, "-", "Deposit", amount);
                }
                break;
            }
            case MENU_WITHDRAW: {
                char accNum[20] = {0};
                double amount = 0.0;
                int code;

                if (ui_readText("Enter Account Number to withdraw: ", accNum, sizeof(accNum)) != SUCCESS ||
                    ui_readDouble("Enter withdrawal amount: ", &amount) != SUCCESS) {
                    printf("Error: Invalid input!\n");
                    break;
                }
                code = tm_withdraw(&am, accNum, amount);
                ui_printResult(code);
                if (code == SUCCESS) {
                    recordAuditLog(accNum, "-", "Withdraw", amount);
                }
                break;
            }
            case MENU_TRANSFER: {
                char srcAcc[20] = {0};
                char destAcc[20] = {0};
                double amount = 0.0;
                int code;

                if (ui_readText("Enter Source Account Number: ", srcAcc, sizeof(srcAcc)) != SUCCESS ||
                    ui_readText("Enter Destination Account Number: ", destAcc, sizeof(destAcc)) != SUCCESS ||
                    ui_readDouble("Enter transfer amount: ", &amount) != SUCCESS) {
                    printf("Error: Invalid input!\n");
                    break;
                }
                code = tm_transfer(&am, srcAcc, destAcc, amount);
                ui_printResult(code);
                if (code == SUCCESS) {
                    recordAuditLog(srcAcc, destAcc, "Transfer", amount);
                }
                break;
            }
            case MENU_SORT: {
                if (am.count <= 0) {
                    printf("Account list is empty. Nothing to sort.\n");
                    break;
                }
                tm_sortByBalance(&am);
                printf("Accounts sorted by balance (highest to lowest) successfully!\n");
                ui_displayAllAccounts(&am);
                break;
            }
            case MENU_DISPLAY_ALL: {
                ui_displayAllAccounts(&am);
                break;
            }
            case MENU_SAVE: {
                int code = file_saveAccounts(&am, DEFAULT_ACCOUNT_FILE);
                ui_printResult(code);
                if (code == SUCCESS) {
                    printf("Successfully saved %d account(s) to %s.\n", am.count, DEFAULT_ACCOUNT_FILE);
                }
                break;
            }
            case MENU_LOAD: {
                int code = file_loadAccounts(&am, DEFAULT_ACCOUNT_FILE);
                ui_printResult(code);
                if (code == SUCCESS) {
                    printf("Successfully loaded %d account(s) from %s.\n", am.count, DEFAULT_ACCOUNT_FILE);
                }
                break;
            }
            case MENU_EXIT: {
                if (am.count > 0) {
                    file_saveAccounts(&am, DEFAULT_ACCOUNT_FILE);
                    printf("Accounts automatically saved to %s.\n", DEFAULT_ACCOUNT_FILE);
                }
                printf("Exiting system. Goodbye!\n");
                break;
            }
            default:
                printf("Choice not available! Please choose between 0 and 10.\n");
                break;
        }
    } while (choice != MENU_EXIT);

    am_freeAccounts(&am);
    return 0;
}

void ui_displayMainMenu(void) {
    printf("\n=========================================\n");
    printf("   BANK ACCOUNT & TRANSACTION SYSTEM     \n");
    printf("=========================================\n");
    printf("1.  Open New Account\n");
    printf("2.  Update Account Holder Info\n");
    printf("3.  Search Account\n");
    printf("4.  Deposit Money\n");
    printf("5.  Withdraw Money\n");
    printf("6.  Transfer Money\n");
    printf("7.  Sort Accounts by Balance\n");
    printf("8.  Display All Accounts\n");
    printf("9.  Save Accounts to File\n");
    printf("10. Load Accounts from File\n");
    printf("0.  Exit System\n");
    printf("=========================================\n");
    printf("Select option: ");
}

int ui_readText(const char *prompt, char *buf, size_t size) {
    size_t len;
    int ch;

    if (prompt == NULL || buf == NULL || size == 0) return ERROR_INVALID_DATA;

    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL) return ERROR_INVALID_DATA;

    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
        len--;
    } else {
        ch = getchar();
        if (ch != '\n' && ch != EOF) {
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            return ERROR_INVALID_DATA;
        }
    }
    if (len == 0) return ERROR_INVALID_DATA;
    return SUCCESS;
}

int ui_readDouble(const char *prompt, double *outVal) {
    int ch;

    if (prompt == NULL || outVal == NULL) return ERROR_INVALID_DATA;

    printf("%s", prompt);
    if (scanf("%lf", outVal) != 1) {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
        return ERROR_INVALID_DATA;
    }
    ch = getchar();
    if (ch != '\n' && ch != EOF) {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
        return ERROR_INVALID_DATA;
    }
    return SUCCESS;
}

int ui_readAccount(Account *acc) {
    int ch;

    if (acc == NULL) return ERROR_INVALID_DATA;
    memset(acc, 0, sizeof(*acc));

    if (ui_readText("Enter Account Number: ", acc->accountNumber, sizeof(acc->accountNumber)) != SUCCESS)
        return ERROR_INVALID_DATA;
    if (ui_readText("Enter Holder Name: ", acc->holderName, sizeof(acc->holderName)) != SUCCESS)
        return ERROR_INVALID_DATA;
    if (ui_readText("Enter Account Type (Savings/Checking): ", acc->accountType, sizeof(acc->accountType)) != SUCCESS)
        return ERROR_INVALID_DATA;

    printf("Enter Initial Balance: ");
    if (scanf("%lf", &acc->balance) != 1) {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
        return ERROR_INVALID_DATA;
    }
    ch = getchar();
    if (ch != '\n' && ch != EOF) {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
        return ERROR_INVALID_DATA;
    }
    return SUCCESS;
}

void ui_printResult(int code) {
    switch (code) {
        case SUCCESS:
            printf("Operation completed successfully!\n");
            break;
        case ERROR_DUPLICATE_ACCOUNT:
            printf("Error: Duplicate account number!\n");
            break;
        case ERROR_NOT_FOUND:
            printf("Error: Account number or file not found!\n");
            break;
        case ERROR_MEMORY_FULL:
            printf("Error: Memory full! Could not allocate memory.\n");
            break;
        case ERROR_INSUFFICIENT_BALANCE:
            printf("Error: Insufficient balance!\n");
            break;
        case ERROR_INVALID_DATA:
            printf("Error: Invalid data!\n");
            break;
        default:
            printf("Error: Unknown error code (%d).\n", code);
            break;
    }
}

void ui_printAccount(const Account *acc) {
    if (acc == NULL) return;
    printf("Acc No: %-10s | Name: %-20s | Type: %-10s | Balance: %12.2f | Date: %s\n",
           acc->accountNumber, acc->holderName, acc->accountType,
           acc->balance, acc->openingDate);
}

void ui_printSearchResults(const AccountManager *am, const int *indexes, int found) {
    int i;

    if (am == NULL || indexes == NULL) return;

    printf("\n--- SEARCH RESULTS ---\n");
    if (found <= 0) {
        printf("No matching accounts found.\n");
        return;
    }
    for (i = 0; i < found; i++) {
        if (indexes[i] >= 0 && indexes[i] < am->count)
            ui_printAccount(&am->accounts[indexes[i]]);
    }
}

void ui_displayAllAccounts(const AccountManager *am) {
    int i;

    if (am == NULL || am->count <= 0) {
        printf("\nNo accounts in system.\n");
        return;
    }

    printf("\n================================== ALL ACCOUNTS (%d) ==================================\n", am->count);
    for (i = 0; i < am->count; i++) {
        ui_printAccount(&am->accounts[i]);
    }
    printf("======================================================================================\n");
}
