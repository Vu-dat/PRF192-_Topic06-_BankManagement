#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "common.h"

int tm_deposit(AccountManager *am, const char *accNum, double amount);
int tm_withdraw(AccountManager *am, const char *accNum, double amount);
int tm_transfer(AccountManager *am, const char *srcAccNum, const char *destAccNum, double amount);
void tm_sortByBalance(AccountManager *am);
void tm_sortByBalanceOrder(AccountManager *am, int ascending);

#endif
