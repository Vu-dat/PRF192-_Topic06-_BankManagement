#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "common.h"

int am_openAccount(AccountManager *am, const Account *newAcc);
int am_updateHolderInfo(AccountManager *am, const char *accNum, const char *newName);
int am_searchAccount(const AccountManager *am, const char *keyword, int *outIndexes, int maxOut);
int am_deleteAccount(AccountManager *am, const char *accNum);
void am_freeAccounts(AccountManager *am);

#endif
