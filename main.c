#include <stdio.h>
#include <string.h>
#include "common.h"
#include "account.h"
#include "bank_system.h"     

/* ==================== NGUYÊN MẪU HÀM (TRƯỚC main) ==================== */

/* 2. Phân hệ Transaction (Nguyễn Nhật An) - đang phát triển*/
int tm_withdraw(AccountManager *am, const char *accNum, double amount);              /* Rút tiền */
int tm_transfer(AccountManager *am, const char *src, const char *dest, double amt);  /* Chuyển khoản */
void tm_sortByBalance(AccountManager *am);                                           /* Sắp xếp tài khoản theo số dư giảm dần */

/* 3. Phân hệ File I/O (Phạm Nguyễn Tiến Phát) - đang phát triển
int file_saveAccounts(const AccountManager *am, const char *filename);
int file_loadAccounts(AccountManager *am, const char *filename);
*/

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

/* --- Giao diện: CHỈ làm nhập/xuất, không chứa xử lý nghiệp vụ --- */
void ui_displayMainMenu(void);
int  ui_readText(const char *prompt, char *buf, size_t size);
int  ui_readAccount(Account *acc);
void ui_printResult(int code);
void ui_printAccount(const Account *acc);
void ui_printSearchResults(const AccountManager *am, const int *indexes, int found);

/* --- Hằng số: không dùng magic number --- */
#define MENU_NONE          (-1)
#define MENU_EXIT          0
#define MENU_OPEN_ACCOUNT  1
#define MENU_UPDATE_HOLDER 2
#define MENU_SEARCH        3
#define MENU_SORT          4
#define MENU_SAVE          5
#define MENU_LOAD          6
#define MAX_SEARCH_RESULTS 50

/* ==================== ĐỊNH NGHĨA main (các hàm khác định nghĩa SAU main) ==================== */
int main(void) {
    AccountManager am;
    int choice = MENU_NONE;
    int ch;

    am.accounts = NULL;
    am.count = 0;

    do {
        ui_displayMainMenu();
        if (scanf("%d", &choice) != 1) {
            if (feof(stdin)) {                    /* kết thúc nhập liệu -> thoát an toàn */
                printf("\nInput stream closed. Exiting system.\n");
                choice = MENU_EXIT;
                continue;
            }
            printf("Invalid input! Please enter a number.\n");
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }
        while ((ch = getchar()) != '\n' && ch != EOF) { }   /* bỏ phần còn lại của dòng */

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
                if (code == SUCCESS)
                    printf("Opened on: %s\n", am.accounts[am.count - 1].openingDate);
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
                if (found == MAX_SEARCH_RESULTS)
                    printf("Note: only the first %d matches are shown.\n", found);
                break;
            }
            case MENU_SORT:
            case MENU_SAVE:
            case MENU_LOAD:
                printf("Feature under development by team member.\n");
                break;
            case MENU_EXIT:
                printf("Exiting system. Goodbye!\n");
                break;
            default:
                printf("Choice not available yet!\n");
        }
    } while (choice != MENU_EXIT);

    am_freeAccounts(&am);
    return 0;
}

/* ---------------------------------- GIAO DIỆN ---------------------------------- */
void ui_displayMainMenu(void) {
    printf("\n=========================================\n");
    printf("   BANK ACCOUNT & TRANSACTION SYSTEM     \n");
    printf("=========================================\n");
    printf("1. Open New Account\n");
    printf("2. Update Account Holder Info\n");
    printf("3. Search Account\n");
    printf("4. Sort Accounts by Balance\n");
    printf("5. Save Accounts to File\n");
    printf("6. Load Accounts from File\n");
    printf("0. Exit System\n");
    printf("=========================================\n");
    printf("Select option: ");
}

/* Đọc một dòng; trả SUCCESS nếu hợp lệ, ERROR_INVALID_DATA nếu rỗng/quá dài/EOF. */
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
        /* dòng dài hơn bộ đệm: làm sạch phần còn lại rồi báo lỗi (tránh tràn mảng) */
        ch = getchar();
        if (ch != '\n' && ch != EOF) {
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            return ERROR_INVALID_DATA;
        }
    }
    if (len == 0) return ERROR_INVALID_DATA;
    return SUCCESS;
}

/* Đọc đủ 1 tài khoản mới từ bàn phím; không gọi hàm xử lý nào. */
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
    ch = getchar();                                   /* bỏ ký tự xuống dòng còn sót */
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
            printf("Error: Account number not found!\n");
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
    printf("Acc No: %s | Name: %s | Type: %s | Balance: %.2f | Date: %s\n",
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
