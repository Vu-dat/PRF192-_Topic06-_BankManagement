#ifndef TRANSACTION_H    // Kiểm tra nếu TRANSACTION_H chưa được định nghĩa thì mới biên dịch khối bên dưới
#define TRANSACTION_H    // Định nghĩa TRANSACTION_H để chống lỗi định nghĩa lặp lại khi include nhiều lần
 
#include "common.h"      // Dùng các kiểu Account, AccountManager, Transaction trong common.h
 
#define TRANSFER_FEE 0.0                         
// Phí chuyển khoản
#define TRANSACTION_LOG_FILE "transactions.txt"  
// Tên tệp lưu nhật ký giao dịch
#define INDEX_NOT_FOUND -1                       
// Giá trị trả về khi không tìm thấy tài khoản trong mảng
 
#define TX_TYPE_DEPOSIT  "Deposit"               
// Loại giao dịch nạp tiền
#define TX_TYPE_WITHDRAW "Withdraw"              
// Loại giao dịch rút tiền
#define TX_TYPE_TRANSFER "Transfer"              
// Loại giao dịch chuyển khoản



int tm_findAccountIndex(const AccountManager *am, const char *accNum);   // Tìm vị trí tài khoản trong mảng, không thấy trả về INDEX_NOT_FOUND
 
Transaction tm_createTransactionRecord(const char *src, const char *dest, double amount, const char *type);  // Tạo bản ghi giao dịch (nạp, rút thì src và dest giống nhau)
 
int tm_logTransaction(Transaction tx);   // Ghi giao dịch vào tệp log, thành công trả về SUCCESS, lỗi trả về ERROR_INVALID_DATA
 
int tm_deposit(AccountManager *am, const char *accNum, double amount);   // Nạp tiền vào tài khoản accNum
 
int tm_withdraw(AccountManager *am, const char *accNum, double amount);  // Rút tiền từ tài khoản accNum
 
int tm_transfer(AccountManager *am, const char *sourceAccNum, const char *destAccNum, double amount);  // Chuyển khoản, nguồn trừ amount + phí, đích cộng amount
 
void tm_sortByBalance(AccountManager *am);   // Sắp xếp tài khoản theo số dư giảm dần
 
#endif
