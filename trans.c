#define _CRT_SECURE_NO_WARNINGS
#include <_mingw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
 
#define MAX_ACCOUNTS   100
#define MAX_USERS       50
#define MAX_FD          50
#define MAX_ATTEMPTS     3
#define MIN_SAVINGS    500.0
#define MIN_CURRENT   1000.0
#define SAVINGS_RATE     3.0
#define FD_RATE          6.5
#define FD_PENALTY       1.5
 
struct Account
{
    unsigned int acctNum;
    char    lastName[20];
    char    firstName[20];
    char    address[60];
    char    phone[15];
    char    dob[11];
    int     age;
    int     acctType;
    double  balance;
    int     pin;
    int     isActive;
    char    userID[20];
};
 
struct FixedDeposit
{
    int    fdID;
    int    acctNum;
    double principal;
    int    tenureMonths;
    char   startDate[11];
    char   maturityDate[11];
    double maturityAmount;
    int    isActive;
};
 
struct UserLogin
{
    char   userID[20];
    char   password[30];
    int    role;
    int    linkedAcct;
    int    locked;
};
 
void    clearBuffer(void);
int     safeReadInt(void);
double  safeReadDouble(void);
void    getCurrentDateTime(char *buf, int bufLen);
void    getCurrentDate(char *buf, int bufLen);
int     calculateAge(const char *dob);
void    addMonthsToDate(const char *startDate, int months, char *result);
int     validateDOB(const char *dob);
int     validatePhone(const char *phone);
void    printSeparator(void);
void    pressEnterToContinue(void);
void    initializeAllFiles(void);
void    initAccountFile(void);
void    initFDFile(void);
void    initUserFile(void);
int     loginSystem(struct UserLogin *outUser);
void    registerUser(void);
int     changePassword(struct UserLogin *currentUser);
void    lockUser(const char *userID);
void    createAccount(FILE *fPtr, const char *userID, int forceAcct);
void    deleteAccount(FILE *fPtr);
void    displayAllAccounts(FILE *fPtr);
void    searchByName(FILE *fPtr);
void    searchByBalanceRange(FILE *fPtr);
void    sortAccounts(FILE *fPtr);
void    displayAccountDetails(FILE *fPtr, unsigned int acctNum);
int     validAccount(unsigned int account);
void    deposit(FILE *fPtr, unsigned int acctNum);
void    withdraw(FILE *fPtr, unsigned int acctNum);
void    fundTransfer(FILE *fPtr, unsigned int fromAcct);
void    applyInterest(FILE *fPtr);
void    applyServiceCharge(FILE *fPtr);
void    logTransaction(unsigned int acctNum, const char *type,
                        double amount, double newBalance, unsigned int toAcct);
void    viewFullHistory(unsigned int acctNum);
void    miniStatement(unsigned int acctNum);
void    viewAllTransactions(void);
void    createFD(FILE *acctFile, unsigned int acctNum);
void    viewMyFDs(unsigned int acctNum);
void    closeFD(FILE *acctFile, unsigned int acctNum);
void    viewAllFDs(void);
void    atmMenu(FILE *fPtr, unsigned int acctNum);
void    exportToTextFile(FILE *fPtr);
void    accountSummary(FILE *fPtr);
void    backupData(void);
void    restoreData(void);
void    adminMenu(FILE *fPtr, struct UserLogin *admin);
void    customerMenu(FILE *fPtr, struct UserLogin *user);
 
void clearBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
 
int safeReadInt(void)
{
    int x;
    if (scanf("%d", &x) != 1) { clearBuffer(); return -1; }
    clearBuffer();
    return x;
}
 
double safeReadDouble(void)
{
    double x;
    if (scanf("%lf", &x) != 1) { clearBuffer(); return -99999.0; }
    clearBuffer();
    return x;
}
 
void getCurrentDateTime(char *buf, int bufLen)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, bufLen, "%d/%m/%Y %H:%M:%S", tm);
}
 
void getCurrentDate(char *buf, int bufLen)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, bufLen, "%d/%m/%Y", tm);
}
 
int calculateAge(const char *dob)
{
    int day, month, year;
    if (sscanf(dob, "%d/%d/%d", &day, &month, &year) != 3) return -1;
    time_t t = time(NULL);
    struct tm *today = localtime(&t);
    int age = (today->tm_year + 1900) - year;
    if (today->tm_mon + 1 < month ||
        (today->tm_mon + 1 == month && today->tm_mday < day))
        age--;
    return age;
}
 
void addMonthsToDate(const char *startDate, int months, char *result)
{
    int day, month, year;
    sscanf(startDate, "%d/%d/%d", &day, &month, &year);
    month += months;
    while (month > 12) { month -= 12; year++; }
    sprintf(result, "%02d/%02d/%04d", day, month, year);
}
 
int validateDOB(const char *dob)
{
    int day, month, year;
    if (strlen(dob) != 10) return 0;
    if (sscanf(dob, "%d/%d/%d", &day, &month, &year) != 3) return 0;
    if (day < 1 || day > 31) return 0;
    if (month < 1 || month > 12) return 0;
    if (year < 1900 || year > 2025) return 0;
    return 1;
}
 
int validatePhone(const char *phone)
{
    if (strlen(phone) != 10) return 0;
    for (int i = 0; i < 10; i++)
        if (!isdigit((unsigned char)phone[i])) return 0;
    return 1;
}
 
void printSeparator(void)
{
    printf("------------------------------------------------------------\n");
}
 
void pressEnterToContinue(void)
{
    printf("\nPress ENTER to continue...");
    clearBuffer();
}
 
void initAccountFile(void)
{
    FILE *fp = fopen("credit.dat", "wb");
    if (!fp) { printf("Error creating credit.dat\n"); return; }
    struct Account blank;
    memset(&blank, 0, sizeof(struct Account));
    for (int i = 0; i < MAX_ACCOUNTS; i++)
        fwrite(&blank, sizeof(struct Account), 1, fp);
    fclose(fp);
}
 
void initFDFile(void)
{
    FILE *fp = fopen("fd.dat", "wb");
    if (!fp) { printf("Error creating fd.dat\n"); return; }
    struct FixedDeposit blank;
    memset(&blank, 0, sizeof(struct FixedDeposit));
    for (int i = 0; i < MAX_FD; i++)
        fwrite(&blank, sizeof(struct FixedDeposit), 1, fp);
    fclose(fp);
}
 
void initUserFile(void)
{
    FILE *fp = fopen("users.dat", "wb");
    if (!fp) { printf("Error creating users.dat\n"); return; }
    struct UserLogin blank;
    memset(&blank, 0, sizeof(struct UserLogin));
    for (int i = 0; i < MAX_USERS; i++)
        fwrite(&blank, sizeof(struct UserLogin), 1, fp);
    struct UserLogin admin;
    memset(&admin, 0, sizeof(struct UserLogin));
    strcpy(admin.userID, "admin");
    strcpy(admin.password, "admin123");
    admin.role = 1;
    admin.linkedAcct = 0;
    admin.locked = 0;
    rewind(fp);
    fwrite(&admin, sizeof(struct UserLogin), 1, fp);
    fclose(fp);
    printf("Default admin created. Username: admin  Password: admin123\n");
}
 
void initializeAllFiles(void)
{
    FILE *fp;
    fp = fopen("credit.dat", "rb");
    if (!fp) { initAccountFile(); printf("credit.dat initialized.\n"); } else fclose(fp);
    fp = fopen("fd.dat", "rb");
    if (!fp) { initFDFile(); printf("fd.dat initialized.\n"); } else fclose(fp);
    fp = fopen("users.dat", "rb");
    if (!fp) { initUserFile(); printf("users.dat initialized.\n"); } else fclose(fp);
}
 
int loginSystem(struct UserLogin *outUser)
{
    char userID[20], password[30];
    int attempts = 0;
    printSeparator();
    printf("       WELCOME TO BANKING MANAGEMENT SYSTEM\n");
    printSeparator();
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Username: "); scanf("%19s", userID); clearBuffer();
        printf("Password: "); scanf("%29s", password); clearBuffer();
        FILE *fp = fopen("users.dat", "rb");
        if (!fp) { printf("User database not found.\n"); return 0; }
        struct UserLogin u;
        int found = 0;
        while (fread(&u, sizeof(struct UserLogin), 1, fp) == 1)
        {
            if (strcmp(u.userID, userID) == 0)
            {
                found = 1;
                if (u.locked)
                {
                    printf("Account LOCKED. Contact admin.\n");
                    fclose(fp); return 0;
                }
                if (strcmp(u.password, password) == 0)
                {
                    *outUser = u;
                    fclose(fp);
                    printf("\nLogin successful! Welcome, %s.\n", u.userID);
                    return 1;
                }
                break;
            }
        }
        fclose(fp);
        attempts++;
        if (!found) printf("Username not found.\n");
        else printf("Wrong password. Attempts left: %d\n", MAX_ATTEMPTS - attempts);
    }
    lockUser(userID);
    printf("Too many failed attempts. Account locked.\n");
    return 0;
}
 
void lockUser(const char *userID)
{
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) return;
    struct UserLogin u;
    long pos = 0;
    while (fread(&u, sizeof(struct UserLogin), 1, fp) == 1)
    {
        if (strcmp(u.userID, userID) == 0)
        {
            u.locked = 1;
            fseek(fp, pos, SEEK_SET);
            fwrite(&u, sizeof(struct UserLogin), 1, fp);
            break;
        }
        pos += sizeof(struct UserLogin);
    }
    fclose(fp);
}
 
void registerUser(void)
{
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) { printf("Cannot open users.dat\n"); return; }
    struct UserLogin u;
    long emptyPos = -1;
    char newID[20];
    printf("Enter new username: "); scanf("%19s", newID); clearBuffer();
    rewind(fp);
    long pos = 0;
    while (fread(&u, sizeof(struct UserLogin), 1, fp) == 1)
    {
        if (strcmp(u.userID, newID) == 0)
        {
            printf("Username already exists.\n"); fclose(fp); return;
        }
        if (u.userID[0] == '\0' && emptyPos == -1) emptyPos = pos;
        pos += sizeof(struct UserLogin);
    }
    if (emptyPos == -1) { printf("User database full.\n"); fclose(fp); return; }
    memset(&u, 0, sizeof(struct UserLogin));
    strcpy(u.userID, newID);
    printf("Enter password: "); scanf("%29s", u.password); clearBuffer();
    printf("Role (0=customer, 1=admin): ");
    u.role = safeReadInt();
    if (u.role != 0 && u.role != 1) u.role = 0;
    if (u.role == 0)
    {
        printf("Link to account number (1-%d): ", MAX_ACCOUNTS);
        u.linkedAcct = safeReadInt();
    }
    u.locked = 0;
    fseek(fp, emptyPos, SEEK_SET);
    fwrite(&u, sizeof(struct UserLogin), 1, fp);
    fclose(fp);
    printf("User '%s' registered (linked to account #%d).\n", u.userID, u.linkedAcct);
}
 
int changePassword(struct UserLogin *currentUser)
{
    char oldPwd[30], newPwd[30];
    printf("Enter current password: "); scanf("%29s", oldPwd); clearBuffer();
    if (strcmp(oldPwd, currentUser->password) != 0)
    {
        printf("Incorrect current password.\n"); return 0;
    }
    printf("Enter new password: "); scanf("%29s", newPwd); clearBuffer();
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) { printf("Cannot open users.dat\n"); return 0; }
    struct UserLogin u;
    long pos = 0;
    while (fread(&u, sizeof(struct UserLogin), 1, fp) == 1)
    {
        if (strcmp(u.userID, currentUser->userID) == 0)
        {
            strcpy(u.password, newPwd);
            strcpy(currentUser->password, newPwd);
            fseek(fp, pos, SEEK_SET);
            fwrite(&u, sizeof(struct UserLogin), 1, fp);
            printf("Password changed successfully.\n");
            fclose(fp); return 1;
        }
        pos += sizeof(struct UserLogin);
    }
    fclose(fp); return 0;
}
 
int validAccount(unsigned int account)
{
    return (account >= 1 && account <= MAX_ACCOUNTS);
}
 
void createAccount(FILE *fPtr, const char *userID, int forceAcct)
{
    struct Account client;
    unsigned int account;
    printSeparator();
    printf("         CREATE NEW ACCOUNT\n");
    printSeparator();
    if (forceAcct > 0)
    {
        account = (unsigned int)forceAcct;
        printf("Creating account #%u\n", account);
    }
    else
    {
        printf("Enter account number (1-%d): ", MAX_ACCOUNTS);
        account = (unsigned int)safeReadInt();
    }
    if (!validAccount(account)) { printf("Invalid account number.\n"); return; }
    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&client, sizeof(struct Account), 1, fPtr) != 1)
    {
        printf("File read error.\n"); return;
    }
    if (client.isActive == 1) { printf("Account %u already exists.\n", account); return; }
    memset(&client, 0, sizeof(struct Account));
    client.acctNum = account;
    strcpy(client.userID, userID);
    printf("First name: "); scanf("%19s", client.firstName); clearBuffer();
    printf("Last name: ");  scanf("%19s", client.lastName);  clearBuffer();
    printf("Address (use _ for spaces): "); scanf("%59s", client.address); clearBuffer();
    do {
        printf("Phone (10 digits): "); scanf("%14s", client.phone); clearBuffer();
        if (!validatePhone(client.phone)) printf("Invalid phone. Must be 10 digits.\n");
    } while (!validatePhone(client.phone));
    do {
        printf("Date of birth (DD/MM/YYYY): "); scanf("%10s", client.dob); clearBuffer();
        if (!validateDOB(client.dob)) printf("Invalid date. Use DD/MM/YYYY\n");
    } while (!validateDOB(client.dob));
    client.age = calculateAge(client.dob);
    printf("Age calculated: %d years\n", client.age);
    if (client.age < 18)
    {
        client.acctType = 3;
        printf("Minor account will be created (age < 18).\n");
        printf("Initial deposit (min Rs.100): ");
    }
    else
    {
        printf("Account type:\n");
        printf("  1 - Savings (min Rs.%.0f, %.1f%% interest/year)\n", MIN_SAVINGS, SAVINGS_RATE);
        printf("  2 - Current (min Rs.%.0f, service charges apply)\n", MIN_CURRENT);
        printf("Choice: ");
        int typeChoice = safeReadInt();
        client.acctType = (typeChoice == 2) ? 2 : 1;
        printf("Initial deposit (min Rs.%.0f): ",
               client.acctType == 1 ? MIN_SAVINGS : MIN_CURRENT);
    }
    client.balance = safeReadDouble();
    double minRequired = (client.acctType == 1) ? MIN_SAVINGS :
                         (client.acctType == 2) ? MIN_CURRENT : 100.0;
    if (client.balance < minRequired)
    {
        printf("Minimum opening deposit is Rs.%.2f\n", minRequired); return;
    }
    int pin = 0;
    do {
        printf("Set 4-digit ATM PIN: ");
        pin = safeReadInt();
        if (pin < 1000 || pin > 9999) printf("PIN must be exactly 4 digits.\n");
    } while (pin < 1000 || pin > 9999);
    client.pin = pin;
    client.isActive = 1;
    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&client, sizeof(struct Account), 1, fPtr);
    logTransaction(account, "OPEN", client.balance, client.balance, 0);
    printSeparator();
    printf("Account %u created successfully!\n", account);
    printf("Type: %s | Balance: Rs.%.2f\n",
           client.acctType == 1 ? "Savings" :
           client.acctType == 2 ? "Current" : "Minor",
           client.balance);
    printSeparator();
}
 
void displayAccountDetails(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }
    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account %u does not exist.\n", acctNum); return;
    }
    printSeparator();
    printf("  ACCOUNT DETAILS - Account #%u\n", c.acctNum);
    printSeparator();
    printf("Name    : %s %s\n", c.firstName, c.lastName);
    printf("Address : %s\n", c.address);
    printf("Phone   : %s\n", c.phone);
    printf("DOB     : %s  (Age: %d)\n", c.dob, c.age);
    printf("Type    : %s\n",
           c.acctType == 1 ? "Savings" : c.acctType == 2 ? "Current" : "Minor");
    printf("Balance : Rs. %.2f\n", c.balance);
    printf("UserID  : %s\n", c.userID);
    printSeparator();
}
 
void deleteAccount(FILE *fPtr)
{
    unsigned int account;
    printf("Enter account number to close (1-%d): ", MAX_ACCOUNTS);
    account = (unsigned int)safeReadInt();
    if (!validAccount(account)) { printf("Invalid account.\n"); return; }
    struct Account c;
    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account does not exist or already closed.\n"); return;
    }
    printf("Closing account of %s %s. Balance Rs.%.2f forfeited.\n",
           c.firstName, c.lastName, c.balance);
    printf("Confirm? (y/n): ");
    char confirm; scanf(" %c", &confirm); clearBuffer();
    if (toupper(confirm) != 'Y') { printf("Cancelled.\n"); return; }
    logTransaction(account, "CLOSE", -c.balance, 0.0, 0);
    c.isActive = 0; c.balance = 0.0;
    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, fPtr);
    printf("Account %u closed.\n", account);
}
 
void displayAllAccounts(FILE *fPtr)
{
    struct Account c; int found = 0;
    rewind(fPtr); printSeparator();
    printf("%-6s %-12s %-12s %-10s %-12s\n", "Acct", "First Name", "Last Name", "Type", "Balance");
    printSeparator();
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive)
        {
            found = 1;
            printf("%-6u %-12s %-12s %-10s Rs.%-9.2f\n",
                   c.acctNum, c.firstName, c.lastName,
                   c.acctType == 1 ? "Savings" : c.acctType == 2 ? "Current" : "Minor",
                   c.balance);
        }
    }
    if (!found) printf("No active accounts found.\n");
    printSeparator();
}
 
void searchByName(FILE *fPtr)
{
    char name[30]; printf("Enter name to search: "); scanf("%29s", name); clearBuffer();
    struct Account c; int found = 0; rewind(fPtr); printSeparator();
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && (strstr(c.firstName, name) || strstr(c.lastName, name)))
        {
            found = 1;
            printf("Acct#%-5u | %s %s | %s | Rs.%.2f\n",
                   c.acctNum, c.firstName, c.lastName,
                   c.acctType == 1 ? "Savings" : c.acctType == 2 ? "Current" : "Minor",
                   c.balance);
        }
    }
    if (!found) printf("No accounts found with name '%s'.\n", name);
    printSeparator();
}
 
void searchByBalanceRange(FILE *fPtr)
{
    double minBal, maxBal;
    printf("Enter minimum balance: "); minBal = safeReadDouble();
    printf("Enter maximum balance: "); maxBal = safeReadDouble();
    if (minBal > maxBal) { printf("Minimum cannot exceed maximum.\n"); return; }
    struct Account c; int found = 0; rewind(fPtr); printSeparator();
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && c.balance >= minBal && c.balance <= maxBal)
        {
            found = 1;
            printf("Acct#%-5u | %s %s | Rs.%.2f\n",
                   c.acctNum, c.firstName, c.lastName, c.balance);
        }
    }
    if (!found) printf("No accounts in that balance range.\n");
    printSeparator();
}
 
void sortAccounts(FILE *fPtr)
{
    struct Account clients[MAX_ACCOUNTS]; int count = 0; struct Account c;
    rewind(fPtr);
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
        if (c.isActive) clients[count++] = c;
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (clients[j].balance < clients[j + 1].balance)
            {
                struct Account tmp = clients[j]; clients[j] = clients[j + 1]; clients[j + 1] = tmp;
            }
    printSeparator();
    printf("  ACCOUNTS SORTED BY BALANCE (HIGH TO LOW)\n");
    printSeparator();
    printf("%-6s %-12s %-12s %-10s %s\n", "Acct", "First", "Last", "Type", "Balance");
    for (int i = 0; i < count; i++)
        printf("%-6u %-12s %-12s %-10s Rs.%.2f\n",
               clients[i].acctNum, clients[i].firstName, clients[i].lastName,
               clients[i].acctType == 1 ? "Savings" : clients[i].acctType == 2 ? "Current" : "Minor",
               clients[i].balance);
    printSeparator();
}
 
void logTransaction(unsigned int acctNum, const char *type,
                    double amount, double newBalance, unsigned int toAcct)
{
    FILE *fp = fopen("transactions.txt", "a");
    if (!fp) return;
    char dateTime[25]; getCurrentDateTime(dateTime, sizeof(dateTime));
    if (toAcct > 0)
        fprintf(fp, "[%s] | Acct:%-4u | Type:%-12s | Amt:%+10.2f | Bal:%10.2f | To:%u\n",
                dateTime, acctNum, type, amount, newBalance, toAcct);
    else
        fprintf(fp, "[%s] | Acct:%-4u | Type:%-12s | Amt:%+10.2f | Bal:%10.2f\n",
                dateTime, acctNum, type, amount, newBalance);
    fclose(fp);
}
 
void viewFullHistory(unsigned int acctNum)
{
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history found.\n"); return; }
    char line[300], search[20];
    sprintf(search, "Acct:%-4u", acctNum);
    int found = 0; printSeparator();
    printf("  FULL TRANSACTION HISTORY - Account #%u\n", acctNum);
    printSeparator();
    while (fgets(line, sizeof(line), fp))
        if (strstr(line, search)) { printf("%s", line); found = 1; }
    if (!found) printf("No transactions found for account %u.\n", acctNum);
    printSeparator(); fclose(fp);
}
 
void miniStatement(unsigned int acctNum)
{
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history.\n"); return; }
    char lines[1000][300], search[20];
    sprintf(search, "Acct:%-4u", acctNum);
    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), fp) && count < 999)
        if (strstr(lines[count], search)) count++;
    fclose(fp);
    printSeparator();
    printf("  MINI STATEMENT - Account #%u (Last 5)\n", acctNum);
    printSeparator();
    int start = (count > 5) ? count - 5 : 0;
    for (int i = start; i < count; i++) printf("%s", lines[i]);
    if (count == 0) printf("No transactions yet.\n");
    printSeparator();
}
 
void viewAllTransactions(void)
{
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history.\n"); return; }
    char line[300]; printSeparator(); printf("  ALL TRANSACTION HISTORY\n"); printSeparator();
    while (fgets(line, sizeof(line), fp)) printf("%s", line);
    printSeparator(); fclose(fp);
}
 
void deposit(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }
    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    { printf("Account not found.\n"); return; }
    printf("Current balance: Rs.%.2f\n", c.balance);
    printf("Enter deposit amount: Rs. "); double amount = safeReadDouble();
    if (amount <= 0) { printf("Amount must be positive.\n"); return; }
    if (amount > 100000.0) { printf("Max single deposit: Rs.1,00,000\n"); return; }
    c.balance += amount;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, fPtr);
    logTransaction(acctNum, "DEPOSIT", amount, c.balance, 0);
    printf("Deposited Rs.%.2f | New balance: Rs.%.2f\n", amount, c.balance);
}
 
void withdraw(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }
    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    { printf("Account not found.\n"); return; }
    printf("Current balance: Rs.%.2f\n", c.balance);
    double minBalance = (c.acctType == 1) ? MIN_SAVINGS :
                        (c.acctType == 2) ? MIN_CURRENT : 100.0;
    printf("Enter withdrawal amount: Rs. "); double amount = safeReadDouble();
    if (amount <= 0) { printf("Amount must be positive.\n"); return; }
    if (c.acctType == 3 && amount > 500.0)
    { printf("Minor accounts: max withdrawal Rs.500 per transaction.\n"); return; }
    if (c.balance - amount < minBalance)
    {
        printf("Insufficient balance. Min balance: Rs.%.2f\n", minBalance);
        printf("Max you can withdraw: Rs.%.2f\n", c.balance - minBalance); return;
    }
    c.balance -= amount;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, fPtr);
    logTransaction(acctNum, "WITHDRAW", -amount, c.balance, 0);
    printf("Withdrawn Rs.%.2f | New balance: Rs.%.2f\n", amount, c.balance);
}
 
void fundTransfer(FILE *fPtr, unsigned int fromAcct)
{
    if (!validAccount(fromAcct)) { printf("Invalid source account.\n"); return; }
    struct Account sender, receiver;
    fseek(fPtr, (fromAcct - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&sender, sizeof(struct Account), 1, fPtr) != 1 || !sender.isActive)
    { printf("Source account not found.\n"); return; }
    printSeparator();
    printf("  FUND TRANSFER\n");
    printSeparator();
    printf("Your balance: Rs.%.2f\n", sender.balance);
    printf("Enter destination account number: ");
    unsigned int toAcct = (unsigned int)safeReadInt();
    if (!validAccount(toAcct)) { printf("Invalid destination account.\n"); return; }
    if (toAcct == fromAcct) { printf("Cannot transfer to your own account.\n"); return; }
    fseek(fPtr, (toAcct - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&receiver, sizeof(struct Account), 1, fPtr) != 1 || !receiver.isActive)
    { printf("Destination account not found.\n"); return; }
    printf("Recipient  : %s %s\n", receiver.firstName, receiver.lastName);
    printf("Account #  : %u\n", receiver.acctNum);
    printf("Type       : %s\n",
           receiver.acctType == 1 ? "Savings" : receiver.acctType == 2 ? "Current" : "Minor");
    printSeparator();
    printf("Enter transfer amount: Rs. "); double amount = safeReadDouble();
    if (amount <= 0) { printf("Amount must be positive.\n"); return; }
    if (amount > 50000.0) { printf("Max transfer: Rs.50,000\n"); return; }
    double minBalance = (sender.acctType == 1) ? MIN_SAVINGS :
                        (sender.acctType == 2) ? MIN_CURRENT : 100.0;
    if (sender.balance - amount < minBalance)
    {
        printf("Insufficient funds. Max transferable: Rs.%.2f\n", sender.balance - minBalance); return;
    }
    printf("Enter remarks (optional): "); char remarks[60]; scanf("%59s", remarks); clearBuffer();
    printf("\nConfirm sending Rs.%.2f to %s %s? (y/n): ",
           amount, receiver.firstName, receiver.lastName);
    char confirm; scanf(" %c", &confirm); clearBuffer();
    if (toupper(confirm) != 'Y') { printf("Transfer cancelled.\n"); return; }
    sender.balance   -= amount;
    receiver.balance += amount;
    fseek(fPtr, (fromAcct - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&sender, sizeof(struct Account), 1, fPtr);
    fseek(fPtr, (toAcct - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&receiver, sizeof(struct Account), 1, fPtr);
    logTransaction(fromAcct, "TRANSFER_OUT", -amount, sender.balance, toAcct);
    logTransaction(toAcct,   "TRANSFER_IN",   amount, receiver.balance, fromAcct);
    printSeparator();
    printf("Transfer Successful!\n");
    printf("Sent      : Rs.%.2f\n", amount);
    printf("To        : %s %s (Acct#%u)\n", receiver.firstName, receiver.lastName, toAcct);
    printf("Remarks   : %s\n", remarks);
    printf("Balance   : Rs.%.2f\n", sender.balance);
    printSeparator();
}
 
void applyInterest(FILE *fPtr)
{
    struct Account c; int count = 0; rewind(fPtr);
    printf("\nApplying monthly interest (%.1f%% annual) to savings accounts...\n", SAVINGS_RATE);
    long pos = 0;
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && c.acctType == 1)
        {
            double interest = c.balance * (SAVINGS_RATE / 100.0) / 12.0;
            c.balance += interest;
            fseek(fPtr, pos, SEEK_SET);
            fwrite(&c, sizeof(struct Account), 1, fPtr);
            fseek(fPtr, pos + sizeof(struct Account), SEEK_SET);
            logTransaction(c.acctNum, "INTEREST", interest, c.balance, 0);
            printf("Acct#%u | Interest: Rs.%.2f | Balance: Rs.%.2f\n",
                   c.acctNum, interest, c.balance);
            count++;
        }
        pos += sizeof(struct Account);
    }
    printf("Interest applied to %d savings accounts.\n", count);
}
 
void applyServiceCharge(FILE *fPtr)
{
    struct Account c; double charge = 50.0; int count = 0; rewind(fPtr);
    printf("\nApplying service charge (Rs.%.2f) to current accounts...\n", charge);
    long pos = 0;
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && c.acctType == 2)
        {
            if (c.balance - charge >= MIN_CURRENT)
            {
                c.balance -= charge;
                fseek(fPtr, pos, SEEK_SET);
                fwrite(&c, sizeof(struct Account), 1, fPtr);
                fseek(fPtr, pos + sizeof(struct Account), SEEK_SET);
                logTransaction(c.acctNum, "SVC_CHARGE", -charge, c.balance, 0);
                printf("Acct#%u | Charge: Rs.%.2f | Balance: Rs.%.2f\n",
                       c.acctNum, charge, c.balance);
                count++;
            }
            else printf("Acct#%u | Skipped (balance too low)\n", c.acctNum);
        }
        pos += sizeof(struct Account);
    }
    printf("Service charge applied to %d current accounts.\n", count);
}
 
void createFD(FILE *acctFile, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }
    struct Account c;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, acctFile) != 1 || !c.isActive)
    { printf("Account not found.\n"); return; }
    printf("Available balance: Rs.%.2f\n", c.balance);
    printf("Enter FD amount: Rs. "); double principal = safeReadDouble();
    if (principal <= 0) { printf("Amount must be positive.\n"); return; }
    if (principal > c.balance) { printf("Insufficient balance.\n"); return; }
    printf("Enter tenure (months, 1-120): "); int months = safeReadInt();
    if (months < 1 || months > 120) { printf("Invalid tenure.\n"); return; }
    double years = months / 12.0;
    double interest = principal * FD_RATE * years / 100.0;
    double maturity = principal + interest;
    char startDate[11], maturityDate[11];
    getCurrentDate(startDate, sizeof(startDate));
    addMonthsToDate(startDate, months, maturityDate);
    FILE *fdFile = fopen("fd.dat", "rb+");
    if (!fdFile) { printf("Cannot open fd.dat\n"); return; }
    struct FixedDeposit fd; long emptyPos = -1; int newFDID = 1; long pos = 0;
    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (!fd.isActive && emptyPos == -1) emptyPos = pos;
        else if (fd.isActive && fd.fdID >= newFDID) newFDID = fd.fdID + 1;
        pos += sizeof(struct FixedDeposit);
    }
    if (emptyPos == -1) { printf("FD storage full.\n"); fclose(fdFile); return; }
    memset(&fd, 0, sizeof(struct FixedDeposit));
    fd.fdID = newFDID; fd.acctNum = acctNum; fd.principal = principal;
    fd.tenureMonths = months; strcpy(fd.startDate, startDate);
    strcpy(fd.maturityDate, maturityDate); fd.maturityAmount = maturity; fd.isActive = 1;
    fseek(fdFile, emptyPos, SEEK_SET);
    fwrite(&fd, sizeof(struct FixedDeposit), 1, fdFile); fclose(fdFile);
    c.balance -= principal;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, acctFile);
    logTransaction(acctNum, "FD_OPEN", -principal, c.balance, 0);
    printSeparator();
    printf("Fixed Deposit Created!\n");
    printf("FD ID       : %d\n", fd.fdID);
    printf("Principal   : Rs.%.2f\n", principal);
    printf("Tenure      : %d months\n", months);
    printf("Start Date  : %s\n", startDate);
    printf("Maturity    : %s\n", maturityDate);
    printf("At Maturity : Rs.%.2f (%.1f%% p.a.)\n", maturity, FD_RATE);
    printf("Remaining balance: Rs.%.2f\n", c.balance);
    printSeparator();
}
 
void viewMyFDs(unsigned int acctNum)
{
    FILE *fdFile = fopen("fd.dat", "rb");
    if (!fdFile) { printf("No FD data found.\n"); return; }
    struct FixedDeposit fd; int found = 0;
    printSeparator(); printf("  FIXED DEPOSITS - Account #%u\n", acctNum); printSeparator();
    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (fd.isActive && fd.acctNum == (int)acctNum)
        {
            found = 1;
            printf("FD#%d | Rs.%.2f | %d months | Start:%s | Matures:%s | At maturity:Rs.%.2f\n",
                   fd.fdID, fd.principal, fd.tenureMonths,
                   fd.startDate, fd.maturityDate, fd.maturityAmount);
        }
    }
    if (!found) printf("No active FDs for account %u.\n", acctNum);
    printSeparator(); fclose(fdFile);
}
 
void closeFD(FILE *acctFile, unsigned int acctNum)
{
    FILE *fdFile = fopen("fd.dat", "rb+");
    if (!fdFile) { printf("No FD data.\n"); return; }
    printf("Enter FD ID to close: "); int fdID = safeReadInt();
    struct FixedDeposit fd; long pos = 0; int found = 0;
    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (fd.isActive && fd.fdID == fdID && fd.acctNum == (int)acctNum) { found = 1; break; }
        pos += sizeof(struct FixedDeposit);
    }
    if (!found) { printf("FD #%d not found.\n", fdID); fclose(fdFile); return; }
    printf("FD #%d | Rs.%.2f | Matures:%s | At maturity:Rs.%.2f\n",
           fd.fdID, fd.principal, fd.maturityDate, fd.maturityAmount);
    printf("Is this FD mature? (y=mature, n=premature): ");
    char ans; scanf(" %c", &ans); clearBuffer();
    double credited;
    if (toupper(ans) == 'Y') { credited = fd.maturityAmount; printf("Crediting Rs.%.2f\n", credited); }
    else
    {
        double penalty = fd.principal * FD_PENALTY / 100.0;
        credited = fd.principal - penalty;
        printf("Premature penalty: Rs.%.2f | Credited: Rs.%.2f\n", penalty, credited);
    }
    struct Account c;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fread(&c, sizeof(struct Account), 1, acctFile);
    c.balance += credited;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, acctFile);
    fd.isActive = 0;
    fseek(fdFile, pos, SEEK_SET);
    fwrite(&fd, sizeof(struct FixedDeposit), 1, fdFile); fclose(fdFile);
    logTransaction(acctNum, "FD_CLOSE", credited, c.balance, 0);
    printf("FD closed. New balance: Rs.%.2f\n", c.balance);
}
 
void viewAllFDs(void)
{
    FILE *fdFile = fopen("fd.dat", "rb");
    if (!fdFile) { printf("No FD data.\n"); return; }
    struct FixedDeposit fd; int found = 0;
    printSeparator(); printf("  ALL ACTIVE FIXED DEPOSITS\n"); printSeparator();
    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (fd.isActive)
        {
            found = 1;
            printf("FD#%-3d | Acct#%-4d | Rs.%.2f | %d months | Matures:%s | Rs.%.2f\n",
                   fd.fdID, fd.acctNum, fd.principal,
                   fd.tenureMonths, fd.maturityDate, fd.maturityAmount);
        }
    }
    if (!found) printf("No active FDs.\n");
    printSeparator(); fclose(fdFile);
}
 
void atmMenu(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }
    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    { printf("Account not found.\n"); return; }
    int attempts = 0, authenticated = 0;
    printSeparator(); printf("  WELCOME TO ATM\n"); printSeparator();
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter 4-digit PIN: "); int enteredPIN = safeReadInt();
        if (enteredPIN == c.pin) { authenticated = 1; break; }
        attempts++;
        printf("Wrong PIN. Attempts left: %d\n", MAX_ATTEMPTS - attempts);
    }
    if (!authenticated) { printf("Card blocked.\n"); return; }
    int atmChoice;
    do {
        printSeparator();
        printf("  ATM | Acct#%u | %s %s\n", c.acctNum, c.firstName, c.lastName);
        printSeparator();
        printf("1-Withdraw  2-Deposit  3-Balance  4-Mini Statement  5-Exit\n: ");
        atmChoice = safeReadInt();
        fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
        fread(&c, sizeof(struct Account), 1, fPtr);
        if (atmChoice == 1)
        {
            printf("Amount (x100, max Rs.10000): Rs. "); double amt = safeReadDouble();
            if ((int)amt % 100 != 0) { printf("Must be multiple of 100.\n"); continue; }
            if (amt > 10000.0) { printf("ATM limit: Rs.10,000\n"); continue; }
            double minBal = (c.acctType == 1) ? MIN_SAVINGS : (c.acctType == 2) ? MIN_CURRENT : 100.0;
            if (c.balance - amt < minBal) { printf("Insufficient. Max: Rs.%.2f\n", c.balance - minBal); continue; }
            c.balance -= amt;
            fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
            fwrite(&c, sizeof(struct Account), 1, fPtr);
            logTransaction(acctNum, "ATM_WITHDRAW", -amt, c.balance, 0);
            printf("Please collect Rs.%.0f | Balance: Rs.%.2f\n", amt, c.balance);
        }
        else if (atmChoice == 2)
        {
            printf("Insert amount: Rs. "); double amt = safeReadDouble();
            if (amt <= 0) { printf("Invalid.\n"); continue; }
            c.balance += amt;
            fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
            fwrite(&c, sizeof(struct Account), 1, fPtr);
            logTransaction(acctNum, "ATM_DEPOSIT", amt, c.balance, 0);
            printf("Deposited Rs.%.2f | Balance: Rs.%.2f\n", amt, c.balance);
        }
        else if (atmChoice == 3) printf("Available Balance: Rs.%.2f\n", c.balance);
        else if (atmChoice == 4) miniStatement(acctNum);
        else if (atmChoice == 5) printf("Thank you. Goodbye!\n");
        else printf("Invalid choice.\n");
    } while (atmChoice != 5);
}
 
void accountSummary(FILE *fPtr)
{
    struct Account c; int total=0, savings=0, current=0, minor=0;
    double totalBal=0, savBal=0, curBal=0, minBal2=0;
    rewind(fPtr);
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive)
        {
            total++; totalBal += c.balance;
            if (c.acctType == 1) { savings++; savBal += c.balance; }
            if (c.acctType == 2) { current++; curBal += c.balance; }
            if (c.acctType == 3) { minor++;   minBal2 += c.balance; }
        }
    }
    printSeparator(); printf("  ACCOUNT SUMMARY\n"); printSeparator();
    printf("Total Accounts : %d\n", total);
    printf("  Savings      : %d  (Rs.%.2f)\n", savings, savBal);
    printf("  Current      : %d  (Rs.%.2f)\n", current, curBal);
    printf("  Minor        : %d  (Rs.%.2f)\n", minor, minBal2);
    printf("Bank Total     : Rs.%.2f\n", totalBal);
    printf("Average Bal    : Rs.%.2f\n", total > 0 ? totalBal / total : 0.0);
    printSeparator();
}
 
void exportToTextFile(FILE *fPtr)
{
    FILE *wp = fopen("accounts.txt", "w");
    if (!wp) { printf("Cannot create accounts.txt\n"); return; }
    struct Account c; rewind(fPtr);
    fprintf(wp, "%-6s %-12s %-12s %-10s %-15s\n", "Acct", "First", "Last", "Type", "Balance");
    fprintf(wp, "------------------------------------------------------------\n");
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
        if (c.isActive)
            fprintf(wp, "%-6u %-12s %-12s %-10s Rs.%.2f\n",
                    c.acctNum, c.firstName, c.lastName,
                    c.acctType == 1 ? "Savings" : c.acctType == 2 ? "Current" : "Minor",
                    c.balance);
    fclose(wp); printf("accounts.txt created successfully.\n");
}
 
void backupData(void)
{
    const char *files[]   = {"credit.dat","fd.dat","users.dat","transactions.txt"};
    const char *backups[] = {"backup_credit.dat","backup_fd.dat","backup_users.dat","backup_transactions.txt"};
    printf("\nBacking up...\n");
    for (int i = 0; i < 4; i++)
    {
        FILE *src = fopen(files[i], "rb"); if (!src) { printf("Skipping %s\n", files[i]); continue; }
        FILE *dst = fopen(backups[i], "wb"); if (!dst) { fclose(src); continue; }
        char buf[512]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), src)) > 0) fwrite(buf, 1, n, dst);
        fclose(src); fclose(dst); printf("Backed up: %s\n", files[i]);
    }
    printf("Backup complete.\n");
}
 
void restoreData(void)
{
    printf("WARNING: This overwrites current data! Continue? (y/n): ");
    char ans; scanf(" %c", &ans); clearBuffer();
    if (toupper(ans) != 'Y') { printf("Cancelled.\n"); return; }
    const char *backups[] = {"backup_credit.dat","backup_fd.dat","backup_users.dat","backup_transactions.txt"};
    const char *files[]   = {"credit.dat","fd.dat","users.dat","transactions.txt"};
    for (int i = 0; i < 4; i++)
    {
        FILE *src = fopen(backups[i], "rb"); if (!src) { printf("Backup not found: %s\n", backups[i]); continue; }
        FILE *dst = fopen(files[i], "wb"); if (!dst) { fclose(src); continue; }
        char buf[512]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), src)) > 0) fwrite(buf, 1, n, dst);
        fclose(src); fclose(dst); printf("Restored: %s\n", files[i]);
    }
    printf("Restore complete. Restart program.\n");
}
 
void adminMenu(FILE *fPtr, struct UserLogin *admin)
{
    int choice;
    do {
        printSeparator();
        printf("  ADMIN MENU | Logged in: %s\n", admin->userID);
        printSeparator();
        printf(" 1  - Create account\n");
        printf(" 2  - Delete/close account\n");
        printf(" 3  - View all accounts\n");
        printf(" 4  - Search by name\n");
        printf(" 5  - Search by balance range\n");
        printf(" 6  - Sort accounts by balance\n");
        printf(" 7  - Account summary report\n");
        printf(" 8  - Export accounts to text file\n");
        printf(" 9  - View all transactions\n");
        printf("10  - Apply monthly interest (savings)\n");
        printf("11  - Apply service charge (current)\n");
        printf("12  - View all fixed deposits\n");
        printf("13  - Register new user\n");
        printf("14  - Backup all data\n");
        printf("15  - Restore from backup\n");
        printf("16  - Change my password\n");
        printf("17  - View account details\n");
        printf(" 0  - Logout\n");
        printf("Choice: "); choice = safeReadInt();
        unsigned int acct;
        switch (choice)
        {
        case 1:  createAccount(fPtr, admin->userID, 0); break;
        case 2:  deleteAccount(fPtr); break;
        case 3:  displayAllAccounts(fPtr); break;
        case 4:  searchByName(fPtr); break;
        case 5:  searchByBalanceRange(fPtr); break;
        case 6:  sortAccounts(fPtr); break;
        case 7:  accountSummary(fPtr); break;
        case 8:  exportToTextFile(fPtr); break;
        case 9:  viewAllTransactions(); break;
        case 10: applyInterest(fPtr); break;
        case 11: applyServiceCharge(fPtr); break;
        case 12: viewAllFDs(); break;
        case 13: registerUser(); break;
        case 14: backupData(); break;
        case 15: restoreData(); break;
        case 16: changePassword(admin); break;
        case 17:
            printf("Enter account number: "); acct = (unsigned int)safeReadInt();
            displayAccountDetails(fPtr, acct); break;
        case 0: printf("Logging out...\n"); break;
        default: printf("Invalid choice.\n");
        }
        if (choice != 0) pressEnterToContinue();
    } while (choice != 0);
}
 
void customerMenu(FILE *fPtr, struct UserLogin *user)
{
    unsigned int myAcct = (unsigned int)user->linkedAcct;
 
    if (!validAccount(myAcct))
    {
        printf("No account linked to '%s'. Contact admin.\n", user->userID);
        return;
    }
 
    struct Account c;
    fseek(fPtr, (myAcct - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account #%u not found. Contact admin.\n", myAcct);
        return;
    }
 
    int choice;
    do {
        fseek(fPtr, (myAcct - 1) * sizeof(struct Account), SEEK_SET);
        fread(&c, sizeof(struct Account), 1, fPtr);
        printSeparator();
        printf("  %s %s | Acct#%u | Rs.%.2f | %s\n",
               c.firstName, c.lastName, myAcct, c.balance,
               c.acctType == 1 ? "Savings" : c.acctType == 2 ? "Current" : "Minor");
        printSeparator();
        printf(" 1 - View my account details\n");
        printf(" 2 - Deposit\n");
        printf(" 3 - Withdraw\n");
        printf(" 4 - Transfer to another account\n");
        printf(" 5 - ATM simulation\n");
        printf(" 6 - Full transaction history\n");
        printf(" 7 - Mini statement (last 5)\n");
        printf(" 8 - Create Fixed Deposit\n");
        printf(" 9 - View my Fixed Deposits\n");
        printf("10 - Close a Fixed Deposit\n");
        printf("11 - Change password\n");
        printf(" 0 - Logout\n");
        printf("Choice: "); choice = safeReadInt();
        switch (choice)
        {
        case 1:  displayAccountDetails(fPtr, myAcct); break;
        case 2:  deposit(fPtr, myAcct); break;
        case 3:  withdraw(fPtr, myAcct); break;
        case 4:  fundTransfer(fPtr, myAcct); break;
        case 5:  atmMenu(fPtr, myAcct); break;
        case 6:  viewFullHistory(myAcct); break;
        case 7:  miniStatement(myAcct); break;
        case 8:  createFD(fPtr, myAcct); break;
        case 9:  viewMyFDs(myAcct); break;
        case 10: closeFD(fPtr, myAcct); break;
        case 11: changePassword(user); break;
        case 0:  printf("Logging out...\n"); break;
        default: printf("Invalid choice.\n");
        }
        if (choice != 0) pressEnterToContinue();
    } while (choice != 0);
}
 
int main(void)
{
    initializeAllFiles();
    FILE *cfPtr = fopen("credit.dat", "rb+");
    if (!cfPtr) { printf("FATAL: Could not open credit.dat\n"); return 1; }
    struct UserLogin currentUser;
    if (!loginSystem(&currentUser))
    {
        printf("Login failed. Exiting.\n");
        fclose(cfPtr); return 1;
    }
    if (currentUser.role == 1)
        adminMenu(cfPtr, &currentUser);
    else
        customerMenu(cfPtr, &currentUser);
    fclose(cfPtr);
    printf("\nThank you for using Banking Management System. Goodbye!\n");
    return 0;
}