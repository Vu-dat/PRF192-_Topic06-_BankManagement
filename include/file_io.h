#ifndef FILE_IO_H
#define FILE_IO_H

#include "common.h"

int file_saveAccounts(const AccountManager *am, const char *filename);
int file_loadAccounts(AccountManager *am, const char *filename);
int file_logTransaction(const char *filename, const Transaction *txn);
int file_loadTransactions(const char *filename, Transaction **outTxns, int *outCount);
void file_freeTransactions(Transaction *txns);

#endif
