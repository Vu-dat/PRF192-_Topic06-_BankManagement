#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "transaction.h"

#define TX_ID_FORMAT   "TXN%04d"                 
// Định dạng mã giao dịch (TXN0001, TXN0002, ...)
#define TX_TIME_FORMAT "%Y-%m-%d %H:%M:%S"       
// Định dạng thời gian của giao dịch
#define TX_LOG_FORMAT  "%s|%s|%s|%s|%.2f|%s\n"   
// Định dạng một dòng trong tệp log
 
/* Nguyên mẫu các hàm nội bộ (chỉ dùng trong file này) */
static int  tm_countLogLines(void);
static int  tm_isValidAmount(double amount);
static void tm_copyText(char *dest, size_t size, const char *src);
static int  tm_recordTransaction(const char *src, const char *dest, double amount, const char *type);


//________helper_________
int tm_findAccountIndex(const AccountManager *am, const char *accNum) {
    int i;
 
    if (am == NULL || accNum == NULL) return INDEX_NOT_FOUND;
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) return INDEX_NOT_FOUND;
 
    for (i = 0; i < am->count; i++) {
        if (strcmp(am->accounts[i].accountNumber, accNum) == 0) return i;
    }
    return INDEX_NOT_FOUND;
}
 
// Đếm số dòng đang có trong tệp log (dùng để sinh mã giao dịch mới)
static int tm_countLogLines(void) {
    FILE *f = fopen(TRANSACTION_LOG_FILE, "r");
    int lines = 0;
    int ch;
 
    if (f == NULL) return 0;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') lines++;
    }
    fclose(f);
    return lines;
}
 
// Số tiền hợp lệ khi lớn hơn 0 (số không phải số, NaN, cũng bị loại vì phép so sánh cho kết quả sai)
static int tm_isValidAmount(double amount) {
    return amount > 0;
}
 
// Sao chép chuỗi có giới hạn kích thước, luôn kết thúc bằng '\0'
static void tm_copyText(char *dest, size_t size, const char *src) {
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}
 
// Tạo bản ghi giao dịch: tự sinh mã giao dịch và thời gian hiện tại, dữ liệu sai thì trả về bản ghi rỗng
Transaction tm_createTransactionRecord(const char *src, const char *dest, double amount, const char *type) {
    Transaction tx;
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
 
    memset(&tx, 0, sizeof(tx));
    if (src == NULL || dest == NULL || type == NULL) return tx;
 
    snprintf(tx.transactionID, sizeof(tx.transactionID), TX_ID_FORMAT, tm_countLogLines() + 1);
    tm_copyText(tx.sourceAcc, sizeof(tx.sourceAcc), src);
    tm_copyText(tx.destAcc, sizeof(tx.destAcc), dest);
    tm_copyText(tx.type, sizeof(tx.type), type);
    tx.amount = amount;
    if (local != NULL) {
        strftime(tx.timestamp, sizeof(tx.timestamp), TX_TIME_FORMAT, local);
    }
    return tx;
}
 
// Ghi giao dịch vào cuối tệp log, thành công trả về SUCCESS, lỗi trả về ERROR_INVALID_DATA
int tm_logTransaction(Transaction tx) {
    FILE *f = fopen(TRANSACTION_LOG_FILE, "a");
 
    if (f == NULL) return ERROR_INVALID_DATA;
    fprintf(f, TX_LOG_FORMAT,
            tx.transactionID, tx.sourceAcc, tx.destAcc, tx.type, tx.amount, tx.timestamp);
    fclose(f);
    return SUCCESS;
}
 
// Tạo rồi ghi một giao dịch vào log (log lỗi không làm hỏng giao dịch đã thực hiện)
static int tm_recordTransaction(const char *src, const char *dest, double amount, const char *type) {
    Transaction tx = tm_createTransactionRecord(src, dest, amount, type);
    return tm_logTransaction(tx);
}

//__________deposit_________
// Trả về: SUCCESS, ERROR_NOT_FOUND (không thấy tài khoản), ERROR_INVALID_DATA (dữ liệu sai hoặc amount <= 0)
int tm_deposit(AccountManager *am, const char *accNum, double amount) {
    int idx;
 
    if (am == NULL || accNum == NULL) return ERROR_INVALID_DATA;
 
    idx = tm_findAccountIndex(am, accNum);
    if (idx == INDEX_NOT_FOUND) return ERROR_NOT_FOUND;
 
    if (!tm_isValidAmount(amount)) return ERROR_INVALID_DATA;
 
    am->accounts[idx].balance = am->accounts[idx].balance + amount;
    tm_recordTransaction(accNum, accNum, amount, TX_TYPE_DEPOSIT);
    return SUCCESS;
}

//________withdraw________
// Trả về: SUCCESS, ERROR_NOT_FOUND, ERROR_INVALID_DATA (dữ liệu sai hoặc amount <= 0), ERROR_INSUFFICIENT_BALANCE
int tm_withdraw(AccountManager *am, const char *accNum, double amount) {
    int idx;
 
    if (am == NULL || accNum == NULL) return ERROR_INVALID_DATA;
 
    idx = tm_findAccountIndex(am, accNum);
    if (idx == INDEX_NOT_FOUND) return ERROR_NOT_FOUND;
 
    if (!tm_isValidAmount(amount)) return ERROR_INVALID_DATA;
 
    if (am->accounts[idx].balance < amount) return ERROR_INSUFFICIENT_BALANCE;
 
    am->accounts[idx].balance = am->accounts[idx].balance - amount;
    tm_recordTransaction(accNum, accNum, amount, TX_TYPE_WITHDRAW);
    return SUCCESS;
}

//________transfer_________
// Trả về: SUCCESS, ERROR_NOT_FOUND (nguồn hoặc đích không tồn tại), ERROR_INVALID_DATA, ERROR_INSUFFICIENT_BALANCE
int tm_transfer(AccountManager *am, const char *sourceAccNum, const char *destAccNum, double amount) {
    int srcIdx, destIdx;
    double fee = TRANSFER_FEE;
 
    if (am == NULL || sourceAccNum == NULL || destAccNum == NULL) return ERROR_INVALID_DATA;
 
    srcIdx = tm_findAccountIndex(am, sourceAccNum);
    if (srcIdx == INDEX_NOT_FOUND) return ERROR_NOT_FOUND;
 
    destIdx = tm_findAccountIndex(am, destAccNum);
    if (destIdx == INDEX_NOT_FOUND) return ERROR_NOT_FOUND;
 
    if (!tm_isValidAmount(amount)) return ERROR_INVALID_DATA;
 
    if (am->accounts[srcIdx].balance < amount + fee) return ERROR_INSUFFICIENT_BALANCE;
 
    am->accounts[srcIdx].balance = am->accounts[srcIdx].balance - (amount + fee);
    am->accounts[destIdx].balance = am->accounts[destIdx].balance + amount;
    tm_recordTransaction(sourceAccNum, destAccNum, amount, TX_TYPE_TRANSFER);
    return SUCCESS;
}
 

//____________short by balance____________
// Sắp xếp giảm dần theo số dư (số dư lớn nhất đứng đầu)
void tm_sortByBalance(AccountManager *am) {
    int i, j;
    Account temp;
 
    if (am == NULL || am->accounts == NULL || am->count < 2) return;
 
    for (i = 0; i < am->count - 1; i++) {
        for (j = i + 1; j < am->count; j++) {
            if (am->accounts[i].balance < am->accounts[j].balance) {
                temp = am->accounts[i];
                am->accounts[i] = am->accounts[j];
                am->accounts[j] = temp;
            }
        }
    }
}
