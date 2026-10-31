#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "file_io.h"

#define MAX_LINE_BUFFER 512

int file_saveAccounts(const AccountManager *am, const char *filename) {
    FILE *fp;
    int i;

    if (am == NULL || filename == NULL || filename[0] == '\0') {
        return ERROR_INVALID_DATA;
    }
    if (am->count < 0 || (am->count > 0 && am->accounts == NULL)) {
        return ERROR_INVALID_DATA;
    }

    fp = fopen(filename, "w");
    if (fp == NULL) {
        char altPath[256];
        if (strstr(filename, "data/") == NULL && strstr(filename, "data\\") == NULL) {
            snprintf(altPath, sizeof(altPath), "data/%s", filename);
            fp = fopen(altPath, "w");
        }
        if (fp == NULL) {
            return ERROR_NOT_FOUND;
        }
    }

    for (i = 0; i < am->count; i++) {
        fprintf(fp, "%s,%s,%s,%.2f,%s\n",
                am->accounts[i].accountNumber,
                am->accounts[i].holderName,
                am->accounts[i].accountType,
                am->accounts[i].balance,
                am->accounts[i].openingDate);
    }

    fflush(fp);
    fclose(fp);
    return SUCCESS;
}

int file_loadAccounts(AccountManager *am, const char *filename) {
    FILE *fp;
    char line[MAX_LINE_BUFFER];

    if (am == NULL || filename == NULL || filename[0] == '\0') {
        return ERROR_INVALID_DATA;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        char altPath[256];
        if (strstr(filename, "data/") == NULL && strstr(filename, "data\\") == NULL) {
            snprintf(altPath, sizeof(altPath), "data/%s", filename);
            fp = fopen(altPath, "r");
        }
        if (fp == NULL) {
            return ERROR_NOT_FOUND;
        }
    }

    if (am->accounts != NULL) {
        free(am->accounts);
        am->accounts = NULL;
    }
    am->count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        Account tempAcc;
        Account *tempPtr;
        size_t len;
        int itemsRead;
        int isDuplicate = 0;
        int i;

        len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        if (len == 0 || line[0] == '#') {
            continue;
        }

        memset(&tempAcc, 0, sizeof(tempAcc));

        if (strchr(line, '|') != NULL) {
            itemsRead = sscanf(line, "%19[^|]|%49[^|]|%19[^|]|%lf|%14s",
                               tempAcc.accountNumber,
                               tempAcc.holderName,
                               tempAcc.accountType,
                               &tempAcc.balance,
                               tempAcc.openingDate);
        } else {
            itemsRead = sscanf(line, "%19[^,],%49[^,],%19[^,],%lf,%14s",
                               tempAcc.accountNumber,
                               tempAcc.holderName,
                               tempAcc.accountType,
                               &tempAcc.balance,
                               tempAcc.openingDate);
        }

        if (itemsRead < 5) {
            continue;
        }

        if (tempAcc.accountNumber[0] == '\0' || tempAcc.holderName[0] == '\0' || tempAcc.balance < 0.0) {
            continue;
        }

        for (i = 0; i < am->count; i++) {
            if (strcmp(am->accounts[i].accountNumber, tempAcc.accountNumber) == 0) {
                isDuplicate = 1;
                break;
            }
        }
        if (isDuplicate) {
            continue;
        }

        tempPtr = (Account *)realloc(am->accounts, (size_t)(am->count + 1) * sizeof(Account));
        if (tempPtr == NULL) {
            fclose(fp);
            return ERROR_MEMORY_FULL;
        }

        am->accounts = tempPtr;
        am->accounts[am->count] = tempAcc;
        am->count++;
    }

    fclose(fp);
    return SUCCESS;
}

int file_logTransaction(const char *filename, const Transaction *txn) {
    FILE *fp;

    if (filename == NULL || filename[0] == '\0' || txn == NULL) {
        return ERROR_INVALID_DATA;
    }

    fp = fopen(filename, "a");
    if (fp == NULL) {
        char altPath[256];
        if (strstr(filename, "data/") == NULL && strstr(filename, "data\\") == NULL) {
            snprintf(altPath, sizeof(altPath), "data/%s", filename);
            fp = fopen(altPath, "a");
        }
        if (fp == NULL) {
            return ERROR_NOT_FOUND;
        }
    }

    fprintf(fp, "%s,%s,%s,%s,%.2f,%s\n",
            txn->transactionID,
            txn->sourceAcc,
            txn->destAcc,
            txn->type,
            txn->amount,
            txn->timestamp);

    fflush(fp);
    fclose(fp);
    return SUCCESS;
}

int file_loadTransactions(const char *filename, Transaction **outTxns, int *outCount) {
    FILE *fp;
    char line[MAX_LINE_BUFFER];
    Transaction *txns = NULL;
    int count = 0;

    if (filename == NULL || filename[0] == '\0' || outTxns == NULL || outCount == NULL) {
        return ERROR_INVALID_DATA;
    }
    *outTxns = NULL;
    *outCount = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        char altPath[256];
        if (strstr(filename, "data/") == NULL && strstr(filename, "data\\") == NULL) {
            snprintf(altPath, sizeof(altPath), "data/%s", filename);
            fp = fopen(altPath, "r");
        }
        if (fp == NULL) {
            return ERROR_NOT_FOUND;
        }
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        Transaction tempTxn;
        Transaction *tempPtr;
        size_t len = strlen(line);
        int itemsRead;

        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        if (len == 0 || line[0] == '#') {
            continue;
        }

        memset(&tempTxn, 0, sizeof(tempTxn));

        if (strchr(line, '|') != NULL) {
            itemsRead = sscanf(line, "%19[^|]|%19[^|]|%19[^|]|%14[^|]|%lf|%24[^\n]",
                               tempTxn.transactionID,
                               tempTxn.sourceAcc,
                               tempTxn.destAcc,
                               tempTxn.type,
                               &tempTxn.amount,
                               tempTxn.timestamp);
        } else {
            itemsRead = sscanf(line, "%19[^,],%19[^,],%19[^,],%14[^,],%lf,%24[^\n]",
                               tempTxn.transactionID,
                               tempTxn.sourceAcc,
                               tempTxn.destAcc,
                               tempTxn.type,
                               &tempTxn.amount,
                               tempTxn.timestamp);
        }

        if (itemsRead < 6) {
            continue;
        }

        tempPtr = (Transaction *)realloc(txns, (size_t)(count + 1) * sizeof(Transaction));
        if (tempPtr == NULL) {
            free(txns);
            fclose(fp);
            return ERROR_MEMORY_FULL;
        }

        txns = tempPtr;
        txns[count] = tempTxn;
        count++;
    }

    fclose(fp);
    *outTxns = txns;
    *outCount = count;
    return SUCCESS;
}

void file_freeTransactions(Transaction *txns) {
    if (txns != NULL) {
        free(txns);
    }
}
