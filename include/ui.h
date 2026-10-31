#ifndef UI_H
#define UI_H

#include <stddef.h>
#include "common.h"

/* ========================================================================= */
/* ANSI Escape Codes for Professional Terminal Formatting                    */
/* ========================================================================= */
#define UI_RESET              "\033[0m"
#define UI_BOLD               "\033[1m"
#define UI_DIM                "\033[2m"
#define UI_UNDERLINE          "\033[4m"

#define UI_COLOR_RESET        "\033[0m"
#define UI_COLOR_BOLD         "\033[1m"
#define UI_COLOR_DIM          "\033[2m"
#define UI_COLOR_UNDERLINE    "\033[4m"

/* Standard Colors */
#define UI_COLOR_RED          "\033[31m"
#define UI_COLOR_GREEN        "\033[32m"
#define UI_COLOR_YELLOW       "\033[33m"
#define UI_COLOR_BLUE         "\033[34m"
#define UI_COLOR_MAGENTA      "\033[35m"
#define UI_COLOR_CYAN         "\033[36m"
#define UI_COLOR_WHITE        "\033[37m"

/* High-Intensity / Bright Colors */
#define UI_BRIGHT_RED         "\033[91m"
#define UI_BRIGHT_GREEN       "\033[92m"
#define UI_BRIGHT_YELLOW      "\033[93m"
#define UI_BRIGHT_BLUE        "\033[94m"
#define UI_BRIGHT_MAGENTA     "\033[95m"
#define UI_BRIGHT_CYAN        "\033[96m"
#define UI_BRIGHT_WHITE       "\033[97m"

/* Default Admin Security Credentials */
#define UI_DEFAULT_ADMIN_PIN  "1234"

/* Role Identifiers */
#define ROLE_EXIT             0
#define ROLE_ADMIN            1
#define ROLE_USER             2

/* Admin Menu Identifiers */
#define ADMIN_MENU_ALL_ACCOUNTS   1
#define ADMIN_MENU_SEARCH         2
#define ADMIN_MENU_SORT           3
#define ADMIN_MENU_STATS          4
#define ADMIN_MENU_TRANSACTIONS   5
#define ADMIN_MENU_DELETE_ACC     6
#define ADMIN_MENU_SAVE           7
#define ADMIN_MENU_LOAD           8
#define ADMIN_MENU_LOGOUT         9
#define ADMIN_MENU_EXIT           0

/* User Menu Identifiers */
#define USER_MENU_OPEN_ACC        1
#define USER_MENU_UPDATE_HOLDER   2
#define USER_MENU_DEPOSIT         3
#define USER_MENU_WITHDRAW        4
#define USER_MENU_TRANSFER        5
#define USER_MENU_INQUIRY         6
#define USER_MENU_LOGOUT          7
#define USER_MENU_EXIT            0

/* Console Initialization & Screen Management */
void ui_initConsole(void);
void ui_clearScreen(void);
void ui_pauseScreen(void);

/* Role & Main Menu Displays */
void ui_displayLoginMenu(void);
void ui_displayAdminMenu(void);
void ui_displayUserMenu(void);
void ui_printBanner(const char *title);

/* Status & Feedback Output */
void ui_printResult(int code);
void ui_printSuccess(const char *msg);
void ui_printError(const char *msg);
void ui_printWarning(const char *msg);
void ui_printInfo(const char *msg);

/* Professional Tabular Formats */
void ui_printAccountsTable(const Account *accounts, int count, const char *tableTitle);
void ui_printSearchResultsTable(const AccountManager *am, const int *indexes, int found);
void ui_printTransactionsTable(const Transaction *txns, int count, const char *tableTitle);
void ui_printAccountCard(const Account *acc);
void ui_printStatisticsCard(const AccountManager *am);

/* Robust Console Input Functions */
int  ui_readText(const char *prompt, char *buf, size_t size);
int  ui_readDouble(const char *prompt, double *outVal);
int  ui_readInt(const char *prompt, int *outVal);
int  ui_readAccount(Account *acc);
int  ui_confirmAction(const char *prompt);
int  ui_verifyAdminPin(void);

#endif /* UI_H */
