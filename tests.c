#include <stdio.h>
#include <string.h>
#include "bank_system.h"

static void recordTest(const char *name, int condition, int *passed, int *failed)
{
    if (name == NULL || passed == NULL || failed == NULL) {
        return;
    }
    if (condition) {
        (*passed)++;
        printf("[PASS] %s\n", name);
    } else {
        (*failed)++;
        printf("[FAIL] %s\n", name);
    }
}

static Account makeAccount(const char *number, const char *name, double balance)
{
    Account account = {0};

    snprintf(account.accountNumber, sizeof account.accountNumber, "%s", number);
    snprintf(account.holderName, sizeof account.holderName, "%s", name);
    snprintf(account.accountType, sizeof account.accountType, "Savings");
    account.balance = balance;
    snprintf(account.openingDate, sizeof account.openingDate, "2026-10-01");
    return account;
}

int runTests(void)
{
    AccountManager accounts;
    TransactionManager transactions;
    Account first;
    Account second;
    int firstIndex;
    int testPassed = 0;
    int testFailed = 0;
    initializeAccountManager(&accounts);
    initializeTransactionManager(&transactions);

    first = makeAccount("ACC001", "John", 500.0);
    second = makeAccount("ACC002", "Alice", 100.0);

    recordTest("Create valid account", addAccount(&accounts, &first), &testPassed, &testFailed);
    recordTest("Reject duplicate account", !addAccount(&accounts, &first), &testPassed, &testFailed);
    recordTest("Create second account", addAccount(&accounts, &second), &testPassed, &testFailed);
    recordTest("Find existing account", findAccountIndex(&accounts, "ACC001") == 0, &testPassed, &testFailed);
    recordTest("Reject negative opening balance",
               !addAccount(&accounts, &(Account){
                   "ACC003", "Bad Balance", "Savings", -1.0, "2026-10-01"
               }), &testPassed, &testFailed);

    recordTest("Deposit 100 into ACC001",
               depositMoney(&accounts, &transactions, "ACC001", 100.0,
                            "2026-10-28 10:00"), &testPassed, &testFailed);
    firstIndex = findAccountIndex(&accounts, "ACC001");
    recordTest("Deposit balance becomes 600",
               firstIndex >= 0 && accounts.accounts[firstIndex].balance == 600.0, &testPassed, &testFailed);

    recordTest("Reject negative deposit",
               !depositMoney(&accounts, &transactions, "ACC001", -10.0,
                             "2026-10-28 10:01"), &testPassed, &testFailed);
    recordTest("Withdraw 200 from ACC001",
               withdrawMoney(&accounts, &transactions, "ACC001", 200.0,
                             "2026-10-28 10:02"), &testPassed, &testFailed);
    recordTest("Reject overdraft",
               !withdrawMoney(&accounts, &transactions, "ACC001", 1000.0,
                              "2026-10-28 10:03"), &testPassed, &testFailed);
    recordTest("Transfer 50 from ACC001 to ACC002",
               transferMoney(&accounts, &transactions, "ACC001", "ACC002", 50.0,
                             "2026-10-28 10:04"), &testPassed, &testFailed);
    recordTest("Reject transfer to missing account",
               !transferMoney(&accounts, &transactions, "ACC001", "ACC999", 50.0,
                              "2026-10-28 10:05"), &testPassed, &testFailed);
    recordTest("Reject transfer to same account",
               !transferMoney(&accounts, &transactions, "ACC001", "ACC001", 10.0,
                              "2026-10-28 10:06"), &testPassed, &testFailed);
    recordTest("Reject NULL manager", findAccountIndex(NULL, "ACC001") == -1, &testPassed, &testFailed);
    recordTest("Save and reload accounts",
               saveAccounts(&accounts, "test_accounts_tmp.txt") &&
               loadAccounts(&accounts, "test_accounts_tmp.txt"), &testPassed, &testFailed);
    recordTest("Save and reload transactions",
               saveTransactions(&transactions, "test_transactions_tmp.txt") &&
               loadTransactions(&transactions, "test_transactions_tmp.txt"), &testPassed, &testFailed);

    remove("test_accounts_tmp.txt");
    remove("test_transactions_tmp.txt");
    freeAccountManager(&accounts);
    freeTransactionManager(&transactions);

    printf("\nTest summary: %d passed, %d failed.\n", testPassed, testFailed);
    return testFailed == 0;
}

#ifdef RUN_TESTS
int main(void)
{
    return runTests() ? 0 : 1;
}
#endif
