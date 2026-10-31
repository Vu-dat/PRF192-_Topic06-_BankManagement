#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "account.h"
#include "transaction.h"
#include "file_io.h"
#include "utils.h"
#include "ui.h"

#define DEFAULT_ACCOUNT_FILE "data/accounts.txt"
#define DEFAULT_TRANSACTION_FILE "data/transactions.txt"
#define MAX_SEARCH_RESULTS 50

static void recordAuditLog(const char *srcAcc, const char *destAcc, const char *type, double amount);
static int  findAccountIndex(const AccountManager *am, const char *accNum);
static int  handleAdminSession(AccountManager *am);
static int  handleUserSession(AccountManager *am);

static void recordAuditLog(const char *srcAcc, const char *destAcc, const char *type, double amount) {
    Transaction txn;
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    memset(&txn, 0, sizeof(txn));
    snprintf(txn.transactionID, sizeof(txn.transactionID), "TXN%lu", (unsigned long)now);
    strncpy(txn.sourceAcc, (srcAcc && srcAcc[0] != '\0') ? srcAcc : "-", sizeof(txn.sourceAcc) - 1);
    strncpy(txn.destAcc, (destAcc && destAcc[0] != '\0') ? destAcc : "-", sizeof(txn.destAcc) - 1);
    strncpy(txn.type, (type && type[0] != '\0') ? type : "Unknown", sizeof(txn.type) - 1);
    txn.amount = amount;
    if (local != NULL) {
        strftime(txn.timestamp, sizeof(txn.timestamp), "%Y-%m-%d %H:%M:%S", local);
    } else {
        strncpy(txn.timestamp, "N/A", sizeof(txn.timestamp) - 1);
    }
    file_logTransaction(DEFAULT_TRANSACTION_FILE, &txn);
}

static int findAccountIndex(const AccountManager *am, const char *accNum) {
    int i;
    if (am == NULL || am->accounts == NULL || accNum == NULL) return -1;
    for (i = 0; i < am->count; i++) {
        if (strcmp(am->accounts[i].accountNumber, accNum) == 0) return i;
    }
    return -1;
}

static int handleAdminSession(AccountManager *am) {
    int choice = -1;

    while (1) {
        ui_displayAdminMenu();
        if (ui_readInt("", &choice) != SUCCESS) {
            ui_printError("Invalid input! Please enter a valid menu number.");
            continue;
        }

        switch (choice) {
            case ADMIN_MENU_ALL_ACCOUNTS: {
                ui_printAccountsTable(am->accounts, am->count, "ALL SYSTEM REGISTERED ACCOUNTS");
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_SEARCH: {
                char keyword[64] = {0};
                int indexes[MAX_SEARCH_RESULTS];
                int found;

                if (ui_readText("Enter search keyword (Account No or Holder Name): ", keyword, sizeof(keyword)) != SUCCESS) {
                    ui_printError("Invalid keyword!");
                    break;
                }

                found = am_searchAccount(am, keyword, indexes, MAX_SEARCH_RESULTS);
                ui_printSearchResultsTable(am, indexes, found);
                if (found == MAX_SEARCH_RESULTS) {
                    ui_printInfo("Note: Output capped at maximum 50 matches.");
                }
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_SORT: {
                int sortChoice = 1;

                if (am->count <= 0) {
                    ui_printWarning("Account repository is empty. Nothing to sort.");
                    ui_pauseScreen();
                    break;
                }

                printf("\n" UI_BOLD UI_BRIGHT_CYAN "Choose Sorting Order:\n" UI_COLOR_RESET);
                printf("  [" UI_BRIGHT_YELLOW "1" UI_COLOR_RESET "] Descending (Highest to Lowest Balance)\n");
                printf("  [" UI_BRIGHT_YELLOW "2" UI_COLOR_RESET "] Ascending  (Lowest to Highest Balance)\n");
                printf("Select option [1-2]: ");
                if (ui_readInt("", &sortChoice) != SUCCESS || (sortChoice != 1 && sortChoice != 2)) {
                    ui_printError("Invalid choice! Defaulting to Descending order.");
                    sortChoice = 1;
                }

                if (sortChoice == 1) {
                    tm_sortByBalanceOrder(am, 0);
                    ui_printSuccess("Accounts sorted by balance (Highest -> Lowest)!");
                } else {
                    tm_sortByBalanceOrder(am, 1);
                    ui_printSuccess("Accounts sorted by balance (Lowest -> Highest)!");
                }

                ui_printAccountsTable(am->accounts, am->count, "SORTED ACCOUNTS");
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_STATS: {
                ui_printStatisticsCard(am);
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_TRANSACTIONS: {
                Transaction *allTxns = NULL;
                int txnCount = 0;
                int loadStatus = file_loadTransactions(DEFAULT_TRANSACTION_FILE, &allTxns, &txnCount);

                if (loadStatus == SUCCESS && txnCount > 0) {
                    ui_printTransactionsTable(allTxns, txnCount, "SYSTEM AUDIT TRANSACTION LOGS");
                } else {
                    ui_printWarning("No transaction logs available or file not found.");
                }

                file_freeTransactions(allTxns);
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_DELETE_ACC: {
                char accNum[24] = {0};
                int idx;

                if (ui_readText("Enter Account Number to delete/close: ", accNum, sizeof(accNum)) != SUCCESS) {
                    ui_printError("Invalid account number!");
                    break;
                }

                idx = findAccountIndex(am, accNum);
                if (idx < 0) {
                    ui_printError("Account not found in system!");
                    ui_pauseScreen();
                    break;
                }

                ui_printAccountCard(&am->accounts[idx]);
                if (am->accounts[idx].balance > 0.0) {
                    ui_printWarning("Attention: This account currently has an active balance!");
                }

                if (!ui_confirmAction("Are you sure you want to permanently delete this account? (y/n): ")) {
                    ui_printInfo("Account deletion cancelled by user.");
                    ui_pauseScreen();
                    break;
                }

                if (am_deleteAccount(am, accNum) == SUCCESS) {
                    recordAuditLog(accNum, "-", "CloseAccount", 0.0);
                    ui_printSuccess("Account was permanently closed and deleted from system.");
                    file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                } else {
                    ui_printError("Failed to delete account!");
                }
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_SAVE: {
                int code = file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                ui_printResult(code);
                if (code == SUCCESS) {
                    printf(UI_BRIGHT_GREEN "[SYSTEM] Successfully saved %d account(s) to %s.\n" UI_COLOR_RESET,
                           am->count, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_LOAD: {
                int code = file_loadAccounts(am, DEFAULT_ACCOUNT_FILE);
                ui_printResult(code);
                if (code == SUCCESS) {
                    printf(UI_BRIGHT_GREEN "[SYSTEM] Successfully loaded %d account(s) from %s.\n" UI_COLOR_RESET,
                           am->count, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case ADMIN_MENU_LOGOUT: {
                file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                ui_printInfo("Administrator logged out. Returning to role selection...");
                return ROLE_ADMIN;
            }

            case ADMIN_MENU_EXIT: {
                file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                return ROLE_EXIT;
            }

            default:
                ui_printError("Invalid choice! Please choose an option between 0 and 9.");
                break;
        }
    }
}

static int handleUserSession(AccountManager *am) {
    int choice = -1;

    while (1) {
        ui_displayUserMenu();
        if (ui_readInt("", &choice) != SUCCESS) {
            ui_printError("Invalid input! Please enter a valid menu number.");
            continue;
        }

        switch (choice) {
            case USER_MENU_OPEN_ACC: {
                Account newAcc;
                int code;

                if (ui_readAccount(&newAcc) != SUCCESS) {
                    ui_printError("Account creation aborted due to invalid input.");
                    ui_pauseScreen();
                    break;
                }

                code = am_openAccount(am, &newAcc);
                ui_printResult(code);
                if (code == SUCCESS) {
                    Account *created = &am->accounts[am->count - 1];
                    ui_printAccountCard(created);
                    if (newAcc.balance > 0.0) {
                        recordAuditLog("-", newAcc.accountNumber, "Deposit", newAcc.balance);
                    }
                    file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case USER_MENU_UPDATE_HOLDER: {
                char accNum[24] = {0};
                char newName[64] = {0};
                int idx;
                int code;

                if (ui_readText("Enter Account Number to update: ", accNum, sizeof(accNum)) != SUCCESS) {
                    ui_printError("Invalid account number!");
                    break;
                }

                idx = findAccountIndex(am, accNum);
                if (idx < 0) {
                    ui_printError("Account not found!");
                    ui_pauseScreen();
                    break;
                }

                printf("Current Account Holder: " UI_BOLD UI_BRIGHT_WHITE "%s\n" UI_COLOR_RESET,
                       am->accounts[idx].holderName);

                if (ui_readText("Enter new holder name: ", newName, sizeof(newName)) != SUCCESS ||
                    !utils_isValidName(newName)) {
                    ui_printError("Invalid name! Please enter alphabetic characters.");
                    ui_pauseScreen();
                    break;
                }

                code = am_updateHolderInfo(am, accNum, newName);
                ui_printResult(code);
                if (code == SUCCESS) {
                    ui_printAccountCard(&am->accounts[idx]);
                    file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case USER_MENU_DEPOSIT: {
                char accNum[24] = {0};
                double amount = 0.0;
                int idx;
                int code;

                if (ui_readText("Enter Account Number for deposit: ", accNum, sizeof(accNum)) != SUCCESS) {
                    ui_printError("Invalid account number!");
                    break;
                }

                idx = findAccountIndex(am, accNum);
                if (idx < 0) {
                    ui_printError("Account not found in system!");
                    ui_pauseScreen();
                    break;
                }

                if (ui_readDouble("Enter deposit amount ($ > 0): ", &amount) != SUCCESS || amount <= 0.0) {
                    ui_printError("Deposit amount must be a positive number!");
                    ui_pauseScreen();
                    break;
                }

                code = tm_deposit(am, accNum, amount);
                ui_printResult(code);
                if (code == SUCCESS) {
                    recordAuditLog(accNum, "-", "Deposit", amount);
                    ui_printAccountCard(&am->accounts[idx]);
                    file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case USER_MENU_WITHDRAW: {
                char accNum[24] = {0};
                double amount = 0.0;
                int idx;
                int code;

                if (ui_readText("Enter Account Number for withdrawal: ", accNum, sizeof(accNum)) != SUCCESS) {
                    ui_printError("Invalid account number!");
                    break;
                }

                idx = findAccountIndex(am, accNum);
                if (idx < 0) {
                    ui_printError("Account not found in system!");
                    ui_pauseScreen();
                    break;
                }

                if (ui_readDouble("Enter withdrawal amount ($ > 0): ", &amount) != SUCCESS || amount <= 0.0) {
                    ui_printError("Withdrawal amount must be a positive number!");
                    ui_pauseScreen();
                    break;
                }

                code = tm_withdraw(am, accNum, amount);
                ui_printResult(code);
                if (code == SUCCESS) {
                    recordAuditLog(accNum, "-", "Withdraw", amount);
                    ui_printAccountCard(&am->accounts[idx]);
                    file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case USER_MENU_TRANSFER: {
                char srcAcc[24] = {0};
                char destAcc[24] = {0};
                double amount = 0.0;
                int code;

                if (ui_readText("Enter Source Account Number: ", srcAcc, sizeof(srcAcc)) != SUCCESS ||
                    ui_readText("Enter Destination Account Number: ", destAcc, sizeof(destAcc)) != SUCCESS) {
                    ui_printError("Invalid account number inputs!");
                    break;
                }

                if (ui_readDouble("Enter transfer amount ($ > 0): ", &amount) != SUCCESS || amount <= 0.0) {
                    ui_printError("Transfer amount must be a positive number!");
                    ui_pauseScreen();
                    break;
                }

                code = tm_transfer(am, srcAcc, destAcc, amount);
                ui_printResult(code);
                if (code == SUCCESS) {
                    int srcIdx = findAccountIndex(am, srcAcc);
                    recordAuditLog(srcAcc, destAcc, "Transfer", amount);
                    if (srcIdx >= 0) {
                        printf(UI_BOLD UI_BRIGHT_GREEN "\nTransfer successful! Updated source account status:\n" UI_COLOR_RESET);
                        ui_printAccountCard(&am->accounts[srcIdx]);
                    }
                    file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                }
                ui_pauseScreen();
                break;
            }

            case USER_MENU_INQUIRY: {
                char accNum[24] = {0};
                int idx;
                Transaction *allTxns = NULL;
                Transaction *personalTxns = NULL;
                int totalTxns = 0;
                int userTxnCount = 0;
                int i;

                if (ui_readText("Enter your Account Number to query: ", accNum, sizeof(accNum)) != SUCCESS) {
                    ui_printError("Invalid account number!");
                    break;
                }

                idx = findAccountIndex(am, accNum);
                if (idx < 0) {
                    ui_printError("Account number not found in system!");
                    ui_pauseScreen();
                    break;
                }

                ui_printAccountCard(&am->accounts[idx]);

                if (file_loadTransactions(DEFAULT_TRANSACTION_FILE, &allTxns, &totalTxns) == SUCCESS && totalTxns > 0) {
                    personalTxns = (Transaction *)malloc((size_t)totalTxns * sizeof(Transaction));
                    if (personalTxns != NULL) {
                        for (i = 0; i < totalTxns; i++) {
                            if (strcmp(allTxns[i].sourceAcc, accNum) == 0 ||
                                strcmp(allTxns[i].destAcc, accNum) == 0) {
                                personalTxns[userTxnCount++] = allTxns[i];
                            }
                        }

                        if (userTxnCount > 0) {
                            ui_printTransactionsTable(personalTxns, userTxnCount, "PERSONAL TRANSACTION HISTORY");
                        } else {
                            ui_printInfo("No previous transaction records found for this account.");
                        }
                        free(personalTxns);
                    }
                } else {
                    ui_printInfo("No transaction history recorded in system yet.");
                }

                file_freeTransactions(allTxns);
                ui_pauseScreen();
                break;
            }

            case USER_MENU_LOGOUT: {
                file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                ui_printInfo("Logged out of user session. Returning to role selection...");
                return ROLE_USER;
            }

            case USER_MENU_EXIT: {
                file_saveAccounts(am, DEFAULT_ACCOUNT_FILE);
                return ROLE_EXIT;
            }

            default:
                ui_printError("Invalid choice! Please choose an option between 0 and 7.");
                break;
        }
    }
}

int main(void) {
    AccountManager am;
    int roleChoice = -1;
    int loadStatus;

    ui_initConsole();

    am.accounts = NULL;
    am.count = 0;

    loadStatus = file_loadAccounts(&am, DEFAULT_ACCOUNT_FILE);
    if (loadStatus == SUCCESS && am.count > 0) {
        printf(UI_BRIGHT_GREEN "[SYSTEM] Successfully initialized %d account(s) from %s.\n" UI_COLOR_RESET,
               am.count, DEFAULT_ACCOUNT_FILE);
    } else {
        printf(UI_COLOR_YELLOW "[SYSTEM] No initial account data found in %s. Starting with clean state.\n" UI_COLOR_RESET,
               DEFAULT_ACCOUNT_FILE);
    }

    while (1) {
        ui_displayLoginMenu();
        if (ui_readInt("", &roleChoice) != SUCCESS) {
            ui_printError("Invalid input! Please enter a number (1, 2, or 0).");
            continue;
        }

        if (roleChoice == ROLE_EXIT) {
            break;
        } else if (roleChoice == ROLE_ADMIN) {
            if (ui_verifyAdminPin()) {
                int status = handleAdminSession(&am);
                if (status == ROLE_EXIT) {
                    break;
                }
            }
        } else if (roleChoice == ROLE_USER) {
            int status = handleUserSession(&am);
            if (status == ROLE_EXIT) {
                break;
            }
        } else {
            ui_printError("Unrecognized role option! Please select 1 (Admin), 2 (User), or 0 (Exit).");
        }
    }

    if (am.count > 0) {
        file_saveAccounts(&am, DEFAULT_ACCOUNT_FILE);
        printf(UI_BRIGHT_GREEN "[SYSTEM] All account records safely synchronized to %s.\n" UI_COLOR_RESET, DEFAULT_ACCOUNT_FILE);
    }

    printf("\n" UI_BOLD UI_BRIGHT_CYAN "Thank you for using Bank Account & Transaction System. Goodbye!\n" UI_COLOR_RESET);
    am_freeAccounts(&am);
    return 0;
}
