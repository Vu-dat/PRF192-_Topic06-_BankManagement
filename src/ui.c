#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ui.h"
#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

void ui_initConsole(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    SetConsoleOutputCP(CP_UTF8);
#endif
}

void ui_clearScreen(void) {
    utils_clearScreen();
}

void ui_pauseScreen(void) {
    int ch;
    printf("\n" UI_COLOR_DIM "Press Enter to continue..." UI_COLOR_RESET);
    fflush(stdout);
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

static void printCentered(const char *str, int width) {
    int len = (str != NULL) ? (int)strlen(str) : 0;
    int leftPad;
    int rightPad;

    if (len >= width) {
        printf("%.*s", width, str ? str : "");
        return;
    }

    leftPad = (width - len) / 2;
    rightPad = width - len - leftPad;
    printf("%*s%s%*s", leftPad, "", str ? str : "", rightPad, "");
}

void ui_printBanner(const char *title) {
    int titleLen;
    int totalWidth = 56;
    int padLeft;
    int padRight;
    int i;

    if (title == NULL) title = "BANK MANAGEMENT SYSTEM";
    titleLen = (int)strlen(title);

    printf("\n" UI_BRIGHT_CYAN);
    for (i = 0; i < totalWidth; i++) putchar('=');
    putchar('\n');

    if (titleLen < totalWidth - 4) {
        padLeft = (totalWidth - 2 - titleLen) / 2;
        padRight = totalWidth - 2 - titleLen - padLeft;
        printf(" " UI_BOLD UI_BRIGHT_WHITE "%*s%s%*s" UI_COLOR_RESET UI_BRIGHT_CYAN " \n",
               padLeft, "", title, padRight, "");
    } else {
        printf(" " UI_BOLD UI_BRIGHT_WHITE "%s" UI_COLOR_RESET UI_BRIGHT_CYAN "\n", title);
    }

    for (i = 0; i < totalWidth; i++) putchar('=');
    printf(UI_COLOR_RESET "\n");
}

void ui_displayLoginMenu(void) {
    printf("\n" UI_BRIGHT_CYAN);
    printf("========================================\n");
    printf("   " UI_BOLD UI_BRIGHT_WHITE "BANK MANAGEMENT SYSTEM - LOGIN" UI_COLOR_RESET UI_BRIGHT_CYAN "     \n");
    printf("========================================\n");
    printf(" " UI_BRIGHT_YELLOW "[1]" UI_BRIGHT_WHITE " Administrator (Admin)\n");
    printf(" " UI_BRIGHT_YELLOW "[2]" UI_BRIGHT_WHITE " Regular User (User)\n");
    printf(" " UI_BRIGHT_YELLOW "[0]" UI_COLOR_RED " Exit System\n");
    printf(UI_BRIGHT_CYAN "========================================\n" UI_COLOR_RESET);
    printf(UI_BOLD UI_BRIGHT_CYAN "Enter your role: " UI_COLOR_RESET);
    fflush(stdout);
}

void ui_displayAdminMenu(void) {
    printf("\n" UI_BRIGHT_CYAN);
    printf("========================================================\n");
    printf("         " UI_BOLD UI_BRIGHT_WHITE "ADMINISTRATOR DASHBOARD - CONTROL PANEL" UI_COLOR_RESET UI_BRIGHT_CYAN "        \n");
    printf("========================================================\n");
    printf("  " UI_BRIGHT_YELLOW "[1]" UI_BRIGHT_WHITE " View All Accounts (Grid Table)\n");
    printf("  " UI_BRIGHT_YELLOW "[2]" UI_BRIGHT_WHITE " Global Account Search\n");
    printf("  " UI_BRIGHT_YELLOW "[3]" UI_BRIGHT_WHITE " Sort Accounts by Balance\n");
    printf("  " UI_BRIGHT_YELLOW "[4]" UI_BRIGHT_WHITE " System Financial Statistics\n");
    printf("  " UI_BRIGHT_YELLOW "[5]" UI_BRIGHT_WHITE " View System-Wide Transaction History\n");
    printf("  " UI_BRIGHT_YELLOW "[6]" UI_BRIGHT_WHITE " Delete / Close Account\n");
    printf("  " UI_BRIGHT_YELLOW "[7]" UI_BRIGHT_WHITE " Save Accounts to Disk\n");
    printf("  " UI_BRIGHT_YELLOW "[8]" UI_BRIGHT_WHITE " Reload Accounts from Disk\n");
    printf("  " UI_BRIGHT_YELLOW "[9]" UI_COLOR_YELLOW " Switch Role / Logout to Login Screen\n");
    printf("  " UI_BRIGHT_YELLOW "[0]" UI_COLOR_RED " Exit System\n");
    printf(UI_BRIGHT_CYAN "========================================================\n" UI_COLOR_RESET);
    printf(UI_BOLD UI_BRIGHT_CYAN "Admin Option [0-9]: " UI_COLOR_RESET);
    fflush(stdout);
}

void ui_displayUserMenu(void) {
    printf("\n" UI_BRIGHT_CYAN);
    printf("========================================================\n");
    printf("              " UI_BOLD UI_BRIGHT_WHITE "REGULAR USER - BANKING PORTAL" UI_COLOR_RESET UI_BRIGHT_CYAN "             \n");
    printf("========================================================\n");
    printf("  " UI_BRIGHT_YELLOW "[1]" UI_BRIGHT_WHITE " Open New Account\n");
    printf("  " UI_BRIGHT_YELLOW "[2]" UI_BRIGHT_WHITE " Update Account Holder Info\n");
    printf("  " UI_BRIGHT_YELLOW "[3]" UI_BRIGHT_WHITE " Deposit Money\n");
    printf("  " UI_BRIGHT_YELLOW "[4]" UI_BRIGHT_WHITE " Withdraw Money\n");
    printf("  " UI_BRIGHT_YELLOW "[5]" UI_BRIGHT_WHITE " Transfer Money\n");
    printf("  " UI_BRIGHT_YELLOW "[6]" UI_BRIGHT_WHITE " Account Inquiry & Personal Transaction History\n");
    printf("  " UI_BRIGHT_YELLOW "[7]" UI_COLOR_YELLOW " Switch Role / Logout to Login Screen\n");
    printf("  " UI_BRIGHT_YELLOW "[0]" UI_COLOR_RED " Exit System\n");
    printf(UI_BRIGHT_CYAN "========================================================\n" UI_COLOR_RESET);
    printf(UI_BOLD UI_BRIGHT_CYAN "User Option [0-7]: " UI_COLOR_RESET);
    fflush(stdout);
}

void ui_printSuccess(const char *msg) {
    if (msg == NULL) return;
    printf(UI_BOLD UI_BRIGHT_GREEN "[SUCCESS] " UI_COLOR_RESET UI_BRIGHT_WHITE "%s\n" UI_COLOR_RESET, msg);
}

void ui_printError(const char *msg) {
    if (msg == NULL) return;
    printf(UI_BOLD UI_BRIGHT_RED "[ERROR] " UI_COLOR_RESET UI_COLOR_RED "%s\n" UI_COLOR_RESET, msg);
}

void ui_printWarning(const char *msg) {
    if (msg == NULL) return;
    printf(UI_BOLD UI_BRIGHT_YELLOW "[WARNING] " UI_COLOR_RESET UI_COLOR_YELLOW "%s\n" UI_COLOR_RESET, msg);
}

void ui_printInfo(const char *msg) {
    if (msg == NULL) return;
    printf(UI_BOLD UI_BRIGHT_CYAN "[INFO] " UI_COLOR_RESET UI_BRIGHT_WHITE "%s\n" UI_COLOR_RESET, msg);
}

void ui_printResult(int code) {
    switch (code) {
        case SUCCESS:
            ui_printSuccess("Operation completed successfully!");
            break;
        case ERROR_DUPLICATE_ACCOUNT:
            ui_printError("Account number already exists in system! Please choose another number.");
            break;
        case ERROR_NOT_FOUND:
            ui_printError("Target account or requested data file was not found!");
            break;
        case ERROR_MEMORY_FULL:
            ui_printError("Critical system error: Memory allocation failed!");
            break;
        case ERROR_INSUFFICIENT_BALANCE:
            ui_printError("Transaction rejected: Insufficient account balance!");
            break;
        case ERROR_INVALID_DATA:
            ui_printError("Invalid input parameters! Please check the input values and try again.");
            break;
        default:
            printf(UI_BOLD UI_BRIGHT_RED "[ERROR] " UI_COLOR_RESET UI_COLOR_RED "Encountered unspecified error code (%d).\n" UI_COLOR_RESET, code);
            break;
    }
}

void ui_printAccountsTable(const Account *accounts, int count, const char *tableTitle) {
    int i;
    char balBuf[32];
    char formattedBal[40];

    printf("\n");
    if (tableTitle != NULL && tableTitle[0] != '\0') {
        printf(UI_BOLD UI_BRIGHT_CYAN ">>> %s" UI_COLOR_RESET " (" UI_BRIGHT_YELLOW "%d" UI_COLOR_RESET " account(s))\n",
               tableTitle, count);
    }

    if (accounts == NULL || count <= 0) {
        printf(UI_BRIGHT_CYAN "+----------------------------------------------------------------------------------------------------+\n");
        printf("|" UI_COLOR_RESET UI_COLOR_YELLOW "                                   No accounts found to display.                                    " UI_BRIGHT_CYAN "|\n");
        printf("+----------------------------------------------------------------------------------------------------+\n" UI_COLOR_RESET);
        return;
    }

    /* Table Header Border */
    printf(UI_BRIGHT_CYAN "+----------------+----------------------------+----------------+--------------------+----------------+\n");
    printf("| " UI_BOLD UI_BRIGHT_WHITE "%-14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-26s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%18s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-14s" UI_COLOR_RESET UI_BRIGHT_CYAN " |\n",
           "Account No", "Holder Name", "Type", "Balance", "Opening Date");
    printf("+----------------+----------------------------+----------------+--------------------+----------------+\n" UI_COLOR_RESET);

    /* Table Body Rows */
    for (i = 0; i < count; i++) {
        utils_formatCurrency(accounts[i].balance, balBuf, sizeof(balBuf));
        snprintf(formattedBal, sizeof(formattedBal), "$%s", balBuf);

        printf(UI_BRIGHT_CYAN "| " UI_COLOR_RESET);
        /* Col 1: Account No (Fixed width) */
        printf(UI_BOLD UI_BRIGHT_WHITE "%-14.14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET,
               accounts[i].accountNumber);

        /* Col 2: Holder Name (Left aligned, wide) */
        printf(UI_BRIGHT_WHITE "%-26.26s" UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET,
               accounts[i].holderName);

        /* Col 3: Type (Centered) */
        printf(UI_COLOR_CYAN);
        printCentered(accounts[i].accountType, 14);
        printf(UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET);

        /* Col 4: Balance (Right aligned, currency format) */
        if (accounts[i].balance < 0.0) {
            printf(UI_BOLD UI_BRIGHT_RED "%18s" UI_COLOR_RESET, formattedBal);
        } else {
            printf(UI_BOLD UI_BRIGHT_GREEN "%18s" UI_COLOR_RESET, formattedBal);
        }
        printf(UI_BRIGHT_CYAN " | " UI_COLOR_RESET);

        /* Col 5: Opening Date */
        printf(UI_COLOR_YELLOW "%-14.14s" UI_COLOR_RESET UI_BRIGHT_CYAN " |\n" UI_COLOR_RESET,
               accounts[i].openingDate);
    }

    /* Table Bottom Border */
    printf(UI_BRIGHT_CYAN "+----------------+----------------------------+----------------+--------------------+----------------+\n" UI_COLOR_RESET);
}

void ui_printSearchResultsTable(const AccountManager *am, const int *indexes, int found) {
    Account *tempList;
    int i;

    if (am == NULL || indexes == NULL || found <= 0) {
        ui_printAccountsTable(NULL, 0, "SEARCH RESULTS");
        return;
    }

    tempList = (Account *)malloc((size_t)found * sizeof(Account));
    if (tempList == NULL) {
        ui_printError("Memory allocation failed while displaying search results!");
        return;
    }

    for (i = 0; i < found; i++) {
        if (indexes[i] >= 0 && indexes[i] < am->count) {
            tempList[i] = am->accounts[indexes[i]];
        }
    }

    ui_printAccountsTable(tempList, found, "SEARCH RESULTS");
    free(tempList);
}

void ui_printTransactionsTable(const Transaction *txns, int count, const char *tableTitle) {
    int i;
    char amtBuf[32];
    char formattedAmt[40];

    printf("\n");
    if (tableTitle != NULL && tableTitle[0] != '\0') {
        printf(UI_BOLD UI_BRIGHT_CYAN ">>> %s" UI_COLOR_RESET " (" UI_BRIGHT_YELLOW "%d" UI_COLOR_RESET " transaction(s))\n",
               tableTitle, count);
    }

    if (txns == NULL || count <= 0) {
        printf(UI_BRIGHT_CYAN "+-----------------------------------------------------------------------------------------------------------------+\n");
        printf("|" UI_COLOR_RESET UI_COLOR_YELLOW "                                      No transaction records found.                                               " UI_BRIGHT_CYAN "|\n");
        printf("+-----------------------------------------------------------------------------------------------------------------+\n" UI_COLOR_RESET);
        return;
    }

    /* Border width: 17 + 16 + 16 + 14 + 18 + 23 = 104 */
    printf(UI_BRIGHT_CYAN "+------------------+----------------+----------------+--------------+------------------+-----------------------+\n");
    printf("| " UI_BOLD UI_BRIGHT_WHITE "%-16s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-12s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%16s" UI_COLOR_RESET UI_BRIGHT_CYAN " | "
           UI_BOLD UI_BRIGHT_WHITE "%-21s" UI_COLOR_RESET UI_BRIGHT_CYAN " |\n",
           "Transaction ID", "Source Acc", "Dest Acc", "Type", "Amount", "Timestamp");
    printf("+------------------+----------------+----------------+--------------+------------------+-----------------------+\n" UI_COLOR_RESET);

    for (i = 0; i < count; i++) {
        const char *typeColor = UI_COLOR_WHITE;
        utils_formatCurrency(txns[i].amount, amtBuf, sizeof(amtBuf));
        snprintf(formattedAmt, sizeof(formattedAmt), "$%s", amtBuf);

        if (strcmp(txns[i].type, "Deposit") == 0) {
            typeColor = UI_BRIGHT_GREEN;
        } else if (strcmp(txns[i].type, "Withdraw") == 0) {
            typeColor = UI_BRIGHT_YELLOW;
        } else if (strcmp(txns[i].type, "Transfer") == 0) {
            typeColor = UI_BRIGHT_CYAN;
        } else if (strcmp(txns[i].type, "CloseAccount") == 0) {
            typeColor = UI_BRIGHT_RED;
        }

        printf(UI_BRIGHT_CYAN "| " UI_COLOR_RESET);
        printf(UI_BRIGHT_WHITE "%-16.16s" UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET, txns[i].transactionID);
        printf(UI_COLOR_YELLOW "%-14.14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET, txns[i].sourceAcc);
        printf(UI_COLOR_YELLOW "%-14.14s" UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET, txns[i].destAcc);

        printf("%s", typeColor);
        printCentered(txns[i].type, 12);
        printf(UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET);

        printf(UI_BOLD UI_BRIGHT_GREEN "%16s" UI_COLOR_RESET UI_BRIGHT_CYAN " | " UI_COLOR_RESET, formattedAmt);
        printf(UI_COLOR_DIM "%-21.21s" UI_COLOR_RESET UI_BRIGHT_CYAN " |\n" UI_COLOR_RESET, txns[i].timestamp);
    }

    printf(UI_BRIGHT_CYAN "+------------------+----------------+----------------+--------------+------------------+-----------------------+\n" UI_COLOR_RESET);
}

void ui_printAccountCard(const Account *acc) {
    char balBuf[32];
    char formattedBal[40];

    if (acc == NULL) return;

    utils_formatCurrency(acc->balance, balBuf, sizeof(balBuf));
    snprintf(formattedBal, sizeof(formattedBal), "$%s", balBuf);

    printf("\n" UI_BRIGHT_CYAN "+-------------------------------------------------------------+\n");
    printf("|                " UI_BOLD UI_BRIGHT_WHITE "ACCOUNT INFORMATION SUMMARY" UI_COLOR_RESET UI_BRIGHT_CYAN "                  |\n");
    printf("+-------------------------------------------------------------+\n" UI_COLOR_RESET);
    printf("  " UI_BOLD UI_BRIGHT_CYAN "Account Number : " UI_COLOR_RESET UI_BOLD UI_BRIGHT_WHITE "%s\n" UI_COLOR_RESET, acc->accountNumber);
    printf("  " UI_BOLD UI_BRIGHT_CYAN "Holder Name    : " UI_COLOR_RESET UI_BRIGHT_WHITE "%s\n" UI_COLOR_RESET, acc->holderName);
    printf("  " UI_BOLD UI_BRIGHT_CYAN "Account Type   : " UI_COLOR_RESET UI_COLOR_CYAN "%s\n" UI_COLOR_RESET, acc->accountType);
    printf("  " UI_BOLD UI_BRIGHT_CYAN "Current Balance: " UI_COLOR_RESET UI_BOLD UI_BRIGHT_GREEN "%s\n" UI_COLOR_RESET, formattedBal);
    printf("  " UI_BOLD UI_BRIGHT_CYAN "Opening Date   : " UI_COLOR_RESET UI_COLOR_YELLOW "%s\n" UI_COLOR_RESET, acc->openingDate);
    printf(UI_BRIGHT_CYAN "+-------------------------------------------------------------+\n" UI_COLOR_RESET);
}

void ui_printStatisticsCard(const AccountManager *am) {
    int i;
    double totalBalance = 0.0;
    double avgBalance = 0.0;
    int maxIdx = -1;
    int minIdx = -1;
    int savingsCount = 0;
    int checkingCount = 0;
    double savingsBalance = 0.0;
    double checkingBalance = 0.0;
    char totalBuf[32], avgBuf[32], maxBuf[32], minBuf[32], savBuf[32], chkBuf[32];

    if (am == NULL || am->count <= 0) {
        ui_printWarning("No account data available to generate financial statistics.");
        return;
    }

    maxIdx = 0;
    minIdx = 0;

    for (i = 0; i < am->count; i++) {
        totalBalance += am->accounts[i].balance;

        if (am->accounts[i].balance > am->accounts[maxIdx].balance) {
            maxIdx = i;
        }
        if (am->accounts[i].balance < am->accounts[minIdx].balance) {
            minIdx = i;
        }

        if (strstr(am->accounts[i].accountType, "Saving") != NULL ||
            strstr(am->accounts[i].accountType, "saving") != NULL) {
            savingsCount++;
            savingsBalance += am->accounts[i].balance;
        } else {
            checkingCount++;
            checkingBalance += am->accounts[i].balance;
        }
    }

    avgBalance = totalBalance / am->count;

    utils_formatCurrency(totalBalance, totalBuf, sizeof(totalBuf));
    utils_formatCurrency(avgBalance, avgBuf, sizeof(avgBuf));
    utils_formatCurrency(am->accounts[maxIdx].balance, maxBuf, sizeof(maxBuf));
    utils_formatCurrency(am->accounts[minIdx].balance, minBuf, sizeof(minBuf));
    utils_formatCurrency(savingsBalance, savBuf, sizeof(savBuf));
    utils_formatCurrency(checkingBalance, chkBuf, sizeof(chkBuf));

    printf("\n" UI_BRIGHT_CYAN "+-----------------------------------------------------------------+\n");
    printf("|              " UI_BOLD UI_BRIGHT_WHITE "SYSTEM FINANCIAL & PORTFOLIO STATISTICS" UI_COLOR_RESET UI_BRIGHT_CYAN "            |\n");
    printf("+-----------------------------------------------------------------+\n" UI_COLOR_RESET);
    printf("  " UI_BOLD UI_BRIGHT_WHITE "Overview:\n" UI_COLOR_RESET);
    printf("    * Total Registered Accounts : " UI_BOLD UI_BRIGHT_YELLOW "%d" UI_COLOR_RESET "\n", am->count);
    printf("    * Total System Liquidity   : " UI_BOLD UI_BRIGHT_GREEN "$%s" UI_COLOR_RESET "\n", totalBuf);
    printf("    * Average Account Balance  : " UI_BOLD UI_BRIGHT_CYAN "$%s" UI_COLOR_RESET "\n", avgBuf);
    printf(UI_BRIGHT_CYAN "  ---------------------------------------------------------------\n" UI_COLOR_RESET);
    printf("  " UI_BOLD UI_BRIGHT_WHITE "Extremes:\n" UI_COLOR_RESET);
    printf("    * Highest Balance Account  : " UI_BOLD UI_BRIGHT_WHITE "%s" UI_COLOR_RESET " (%s) -> " UI_BOLD UI_BRIGHT_GREEN "$%s\n" UI_COLOR_RESET,
           am->accounts[maxIdx].accountNumber, am->accounts[maxIdx].holderName, maxBuf);
    printf("    * Lowest Balance Account   : " UI_BOLD UI_BRIGHT_WHITE "%s" UI_COLOR_RESET " (%s) -> " UI_BOLD UI_BRIGHT_YELLOW "$%s\n" UI_COLOR_RESET,
           am->accounts[minIdx].accountNumber, am->accounts[minIdx].holderName, minBuf);
    printf(UI_BRIGHT_CYAN "  ---------------------------------------------------------------\n" UI_COLOR_RESET);
    printf("  " UI_BOLD UI_BRIGHT_WHITE "Distribution By Account Category:\n" UI_COLOR_RESET);
    printf("    * Savings Accounts         : " UI_BRIGHT_YELLOW "%d" UI_COLOR_RESET " accounts (Total: " UI_BRIGHT_GREEN "$%s" UI_COLOR_RESET ")\n",
           savingsCount, savBuf);
    printf("    * Checking Accounts        : " UI_BRIGHT_YELLOW "%d" UI_COLOR_RESET " accounts (Total: " UI_BRIGHT_GREEN "$%s" UI_COLOR_RESET ")\n",
           checkingCount, chkBuf);
    printf(UI_BRIGHT_CYAN "+-----------------------------------------------------------------+\n" UI_COLOR_RESET);
}

int ui_readText(const char *prompt, char *buf, size_t size) {
    size_t len;
    int ch;

    if (prompt == NULL || buf == NULL || size == 0) return ERROR_INVALID_DATA;

    printf("%s", prompt);
    fflush(stdout);
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

    utils_trimWhitespace(buf);
    if (buf[0] == '\0') return ERROR_INVALID_DATA;
    return SUCCESS;
}

int ui_readDouble(const char *prompt, double *outVal) {
    int ch;

    if (prompt == NULL || outVal == NULL) return ERROR_INVALID_DATA;

    printf("%s", prompt);
    fflush(stdout);
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

int ui_readInt(const char *prompt, int *outVal) {
    int ch;

    if (prompt == NULL || outVal == NULL) return ERROR_INVALID_DATA;

    printf("%s", prompt);
    fflush(stdout);
    if (scanf("%d", outVal) != 1) {
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

int ui_confirmAction(const char *prompt) {
    char answer[16] = {0};

    if (prompt == NULL) prompt = "Are you sure? (y/n): ";
    printf(UI_BOLD UI_BRIGHT_YELLOW "%s" UI_COLOR_RESET, prompt);
    fflush(stdout);

    if (ui_readText("", answer, sizeof(answer)) != SUCCESS) {
        return 0;
    }

    if (tolower((unsigned char)answer[0]) == 'y') {
        return 1;
    }
    return 0;
}

int ui_verifyAdminPin(void) {
    char enteredPin[32];
    int attempts = 0;
    const int maxAttempts = 3;

    while (attempts < maxAttempts) {
        printf("\n" UI_BOLD UI_BRIGHT_CYAN "[ADMIN AUTHENTICATION REQUIRED]\n" UI_COLOR_RESET);
        printf("Please enter Administrator Security PIN (Default: " UI_BRIGHT_YELLOW "%s" UI_COLOR_RESET "): ", UI_DEFAULT_ADMIN_PIN);
        fflush(stdout);

        if (ui_readText("", enteredPin, sizeof(enteredPin)) != SUCCESS) {
            ui_printError("Invalid PIN input!");
            attempts++;
            continue;
        }

        if (strcmp(enteredPin, UI_DEFAULT_ADMIN_PIN) == 0) {
            ui_printSuccess("Admin identity verified successfully! Access granted.");
            return 1;
        }

        attempts++;
        printf(UI_BOLD UI_BRIGHT_RED "[ACCESS DENIED] " UI_COLOR_RED "Incorrect PIN! Attempt %d of %d.\n" UI_COLOR_RESET,
               attempts, maxAttempts);
    }

    ui_printError("Maximum login attempts exceeded! Returning to Login Menu.");
    return 0;
}

int ui_readAccount(Account *acc) {
    char typeChoice[16] = {0};

    if (acc == NULL) return ERROR_INVALID_DATA;
    memset(acc, 0, sizeof(*acc));

    printf("\n" UI_BOLD UI_BRIGHT_CYAN "--- ENTER NEW ACCOUNT DETAILS ---\n" UI_COLOR_RESET);

    /* Read and validate Account Number */
    while (1) {
        if (ui_readText("Enter Account Number (4-19 alphanumeric): ", acc->accountNumber, sizeof(acc->accountNumber)) != SUCCESS) {
            ui_printError("Account number cannot be empty!");
            continue;
        }
        if (!utils_isValidAccountNumber(acc->accountNumber)) {
            ui_printError("Invalid account number! Must be 4-19 letters or digits only.");
            continue;
        }
        break;
    }

    /* Read and validate Holder Name */
    while (1) {
        if (ui_readText("Enter Account Holder Name: ", acc->holderName, sizeof(acc->holderName)) != SUCCESS) {
            ui_printError("Holder name cannot be empty!");
            continue;
        }
        if (!utils_isValidName(acc->holderName)) {
            ui_printError("Invalid holder name! Must contain valid alphabetic characters.");
            continue;
        }
        break;
    }

    /* Read and standardize Account Type */
    while (1) {
        printf("Select Account Type [" UI_BRIGHT_YELLOW "1" UI_COLOR_RESET " for Savings, "
               UI_BRIGHT_YELLOW "2" UI_COLOR_RESET " for Checking]: ");
        if (ui_readText("", typeChoice, sizeof(typeChoice)) != SUCCESS) {
            ui_printError("Please enter 1 or 2!");
            continue;
        }
        if (strcmp(typeChoice, "1") == 0 ||
            strcasecmp(typeChoice, "Savings") == 0 ||
            strcasecmp(typeChoice, "Saving") == 0 ||
            strcasecmp(typeChoice, "s") == 0) {
            strncpy(acc->accountType, "Savings", sizeof(acc->accountType) - 1);
            break;
        } else if (strcmp(typeChoice, "2") == 0 ||
                   strcasecmp(typeChoice, "Checking") == 0 ||
                   strcasecmp(typeChoice, "c") == 0) {
            strncpy(acc->accountType, "Checking", sizeof(acc->accountType) - 1);
            break;
        } else {
            ui_printError("Invalid choice! Please select 1 (Savings) or 2 (Checking).");
        }
    }

    /* Read and validate Initial Balance */
    while (1) {
        if (ui_readDouble("Enter Initial Deposit Balance ($ >= 0): ", &acc->balance) != SUCCESS) {
            ui_printError("Invalid number format! Please enter a valid decimal number.");
            continue;
        }
        if (acc->balance < 0.0) {
            ui_printError("Initial balance cannot be negative!");
            continue;
        }
        break;
    }

    return SUCCESS;
}
