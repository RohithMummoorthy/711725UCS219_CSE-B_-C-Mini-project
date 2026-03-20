/*
=============================================================================
  FULL BANKING MANAGEMENT SYSTEM IN C
  Built on top of your original credit.dat program
  Every feature explained with comments for understanding
=============================================================================

HOW THIS FILE IS ORGANIZED:
  1.  Includes & defines
  2.  Structures (customer profile, account, FD, user/login)
  3.  Function prototypes
  4.  Utility functions (safe input, date, validation)
  5.  Login system (admin + customer, PIN, lock)
  6.  File initialization (all data files)
  7.  Customer profile (DOB → age, minor/major, address, contact)
  8.  Account creation (savings / current / minor / FD)
  9.  Deposit & withdraw (with min balance, validation)
  10. Fund transfer (with full logging of from/to)
  11. ATM simulation (PIN, withdraw, deposit, balance, mini statement)
  12. Passbook / transaction history (per account, date+time+type)
  13. Mini statement (last 5 per acco/unt)
  14. Account summary & reports
  15. Search (by name, by balance range) & sort
  16. Backup & restore
  17. Admin menu
  18. Customer menu
  19. Main
=============================================================================
*/

/* ─── 1. INCLUDES & DEFINES ─────────────────────────────────────────────── */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>       /* for date/time in every transaction log */
#include <ctype.h>      /* for toupper(), used in input sanitization */

#define MAX_ACCOUNTS   100   /* maximum number of bank accounts          */
#define MAX_USERS      50    /* maximum admin/customer login users        */
#define MAX_FD         50    /* maximum fixed deposits                    */
#define PIN_LEN        4     /* ATM PIN is exactly 4 digits               */
#define MAX_ATTEMPTS   3     /* login or ATM wrong attempts before lock   */
#define MIN_SAVINGS    500.0 /* minimum balance required for savings acct */
#define MIN_CURRENT    1000.0/* minimum balance required for current acct */
#define SAVINGS_RATE   3.0   /* annual interest rate % for savings        */
#define FD_RATE        6.5   /* annual interest rate % for fixed deposit  */
#define FD_PENALTY     1.5   /* penalty % deducted for premature FD close */

/* ─── 2. STRUCTURES ──────────────────────────────────────────────────────── */

/*
 * ACCOUNT TYPES:
 *   1 = Savings   (interest earned, min balance Rs.500)
 *   2 = Current   (no interest, min balance Rs.1000, service charge)
 *   3 = Minor     (auto-assigned when customer age < 18)
 */
struct Account
{
    unsigned int acctNum;        /* account number 1–100                  */
    char    lastName[20];        /* customer last name                    */
    char    firstName[20];       /* customer first name                   */
    char    address[60];         /* full address                          */
    char    phone[15];           /* contact number                        */
    char    dob[11];             /* date of birth: DD/MM/YYYY             */
    int     age;                 /* auto-calculated from dob at creation  */
    int     acctType;            /* 1=Savings 2=Current 3=Minor           */
    double  balance;             /* current balance                       */
    int     pin;                 /* 4-digit ATM PIN                       */
    int     isActive;            /* 1=active, 0=closed/deleted            */
    char    userID[20];          /* links account to login user           */
};

/*
 * FIXED DEPOSIT STRUCTURE:
 *   Separate from main account. One customer can have multiple FDs.
 *   On maturity: principal + interest. Premature: penalty deducted.
 */
struct FixedDeposit
{
    int    fdID;                 /* unique FD ID 1–50                     */
    int    acctNum;              /* which account this FD belongs to      */
    double principal;            /* amount deposited in FD                */
    int    tenureMonths;         /* duration in months                    */
    char   startDate[11];        /* DD/MM/YYYY when FD was opened         */
    char   maturityDate[11];     /* DD/MM/YYYY calculated at creation     */
    double maturityAmount;       /* principal + interest at maturity      */
    int    isActive;             /* 1=active, 0=closed                    */
};

struct UserLogin
{
    char   userID[20];           /* unique username                       */
    char   password[30];         /* plain-text password (simple system)   */
    int    role;                 /* 0=customer, 1=admin                   */
    int    linkedAcct;           /* which account number this user owns   */
    int    locked;               /* 1 = account locked after 3 fails      */
};

/* ─── 3. FUNCTION PROTOTYPES ─────────────────────────────────────────────── */

/* Utility */
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

/* File init */
void    initializeAllFiles(void);
void    initAccountFile(void);
void    initFDFile(void);
void    initUserFile(void);

/* Login */
int     loginSystem(struct UserLogin *outUser);
void    registerUser(void);
int     changePassword(struct UserLogin *currentUser);
void    lockUser(const char *userID);

/* Account operations */
void    createAccount(FILE *fPtr, const char *userID);
void    deleteAccount(FILE *fPtr);
void    displayAllAccounts(FILE *fPtr);
void    searchByName(FILE *fPtr);
void    searchByBalanceRange(FILE *fPtr);
void    sortAccounts(FILE *fPtr);
void    displayAccountDetails(FILE *fPtr, unsigned int acctNum);
int     validAccount(unsigned int account);

/* Transactions */
void    deposit(FILE *fPtr, unsigned int acctNum);
void    withdraw(FILE *fPtr, unsigned int acctNum);
void    fundTransfer(FILE *fPtr, unsigned int fromAcct);
void    applyInterest(FILE *fPtr);
void    applyServiceCharge(FILE *fPtr);

/* Logging */
void    logTransaction(unsigned int acctNum, const char *type,
                        double amount, double newBalance,
                        unsigned int toAcct);
void    viewFullHistory(unsigned int acctNum);
void    miniStatement(unsigned int acctNum);
void    viewAllTransactions(void);   /* admin only */

/* FD operations */
void    createFD(FILE *acctFile, unsigned int acctNum);
void    viewMyFDs(unsigned int acctNum);
void    closeFD(FILE *acctFile, unsigned int acctNum);
void    viewAllFDs(void);            /* admin only */

/* ATM simulation */
void    atmMenu(FILE *fPtr, unsigned int acctNum);

/* Text file export */
void    exportToTextFile(FILE *fPtr);

/* Account summary */
void    accountSummary(FILE *fPtr);

/* Backup & restore */
void    backupData(void);
void    restoreData(void);

/* Menus */
void    adminMenu(FILE *fPtr, struct UserLogin *admin);
void    customerMenu(FILE *fPtr, struct UserLogin *user);
unsigned int enterAccountNumber(void);


/* ─── 4. UTILITY FUNCTIONS ───────────────────────────────────────────────── */

void clearBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int safeReadInt(void)
{
    int x;
    if (scanf("%d", &x) != 1)
    {
        clearBuffer();
        return -1;
    }
    clearBuffer();
    return x;
}

double safeReadDouble(void)
{
    double x;
    if (scanf("%lf", &x) != 1)
    {
        clearBuffer();
        return -99999.0;   /* sentinel value meaning "bad input" */
    }
    clearBuffer();
    return x;
}
//Date time
void getCurrentDateTime(char *buf, int bufLen)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, bufLen, "%d/%m/%Y %H:%M:%S", tm);
}

//Date 

void getCurrentDate(char *buf, int bufLen)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, bufLen, "%d/%m/%Y", tm);
}

//Age Verification

int calculateAge(const char *dob)
{
    int day, month, year;
    if (sscanf(dob, "%d/%d/%d", &day, &month, &year) != 3)
        return -1; 

    time_t t = time(NULL);
    struct tm *today = localtime(&t);

    int todayYear  = today->tm_year + 1900;
    int todayMonth = today->tm_mon  + 1;
    int todayDay   = today->tm_mday;

    int age = todayYear - year;

    if (todayMonth < month || (todayMonth == month && todayDay < day))
        age--;

    return age;
}

//Adding Month to current date

void addMonthsToDate(const char *startDate, int months, char *result)
{
    int day, month, year;
    sscanf(startDate, "%d/%d/%d", &day, &month, &year);

    month += months;
    while (month > 12)
    {
        month -= 12;
        year++;
    }
    sprintf(result, "%02d/%02d/%04d", day, month, year);
}

// Validating DOB

int validateDOB(const char *dob)
{
    int day, month, year;
    if (strlen(dob) != 10) return 0;
    if (sscanf(dob, "%d/%d/%d", &day, &month, &year) != 3) return 0;
    if (day < 1 || day > 31)    return 0;
    if (month < 1 || month > 12) return 0;
    if (year < 1900 || year > 2025) return 0;
    return 1;
}
//Validating Phone 
int validatePhone(const char *phone)
{
    if (strlen(phone) != 10) return 0;
    for (int i = 0; i < 10; i++)
        if (!isdigit(phone[i])) return 0;
    return 1;
}

/*
 *   Prints a line of dashes for visual formatting in terminal output.
 */
void printSeparator(void)
{
    printf("------------------------------------------------------------\n");
}

/*
 *   Pauses output so user can read before menu re-appears.
 */
void pressEnterToContinue(void)
{
    printf("\nPress ENTER to continue...");
    clearBuffer();
}


/*  5. FILE INITIALIZATION */

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

/*
 *   Creates users.dat and writes a default admin account:
 *   username: admin   password: admin123
 */
void initUserFile(void)
{
    FILE *fp = fopen("users.dat", "wb");
    if (!fp) { printf("Error creating users.dat\n"); return; }

    struct UserLogin blank;
    memset(&blank, 0, sizeof(struct UserLogin));
    for (int i = 0; i < MAX_USERS; i++)
        fwrite(&blank, sizeof(struct UserLogin), 1, fp);

    /* write default admin at slot 0 */
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
    if (!fp) { initAccountFile(); printf("credit.dat initialized.\n"); }
    else fclose(fp);

    fp = fopen("fd.dat", "rb");
    if (!fp) { initFDFile(); printf("fd.dat initialized.\n"); }
    else fclose(fp);

    fp = fopen("users.dat", "rb");
    if (!fp) { initUserFile(); printf("users.dat initialized.\n"); }
    else fclose(fp);

}


/*  6. LOGIN SYSTEM  */

int loginSystem(struct UserLogin *outUser)
{
    char userID[20], password[30];
    int attempts = 0;

    printSeparator();
    printf("       WELCOME TO BANKING MANAGEMENT SYSTEM\n");
    printSeparator();

    while (attempts < MAX_ATTEMPTS)
    {
        printf("Username: ");
        scanf("%19s", userID);
        clearBuffer();
        printf("Password: ");
        scanf("%29s", password);
        clearBuffer();

        FILE *fp = fopen("users.dat", "rb");
        if (!fp)
        {
            printf("User database not found. Run setup first.\n");
            return 0;
        }

        struct UserLogin u;
        int found = 0;

        while (fread(&u, sizeof(struct UserLogin), 1, fp) == 1)
        {
            if (strcmp(u.userID, userID) == 0)
            {
                found = 1;
                if (u.locked)
                {
                    printf("Account is LOCKED after too many failed attempts.\n");
                    printf("Contact admin to unlock.\n");
                    fclose(fp);
                    return 0;
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
        if (!found)
            printf("Username not found.\n");
        else
            printf("Wrong password. Attempts left: %d\n", MAX_ATTEMPTS - attempts);
    }

    
    lockUser(userID);
    printf("Too many failed attempts. Account locked.\n");
    return 0;
}

/*
 *   Reads all users, finds the matching userID, sets locked=1, rewrites.
 *   This is called when MAX_ATTEMPTS wrong passwords are entered.
 */
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

/*
 *   Admin uses this to create a new customer login.
 *   Links the login to an existing account number.
 *   Finds the first empty slot in users.dat (userID[0] == '\0').
 */
void registerUser(void)
{
    FILE *fp = fopen("users.dat", "rb+");
    if (!fp) { printf("Cannot open users.dat\n"); return; }

    struct UserLogin u;
    long emptyPos = -1;
    char newID[20];

    /* check for duplicate username */
    printf("Enter new username: ");
    scanf("%19s", newID);
    clearBuffer();

    rewind(fp);
    long pos = 0;

    while (fread(&u, sizeof(struct UserLogin), 1, fp) == 1)
    {
        if (strcmp(u.userID, newID) == 0)
        {
            printf("Username already exists.\n");
            fclose(fp);
            return;
        }
        if (u.userID[0] == '\0' && emptyPos == -1)
            emptyPos = pos;
        pos += sizeof(struct UserLogin);
    }

    if (emptyPos == -1)
    {
        printf("User database full.\n");
        fclose(fp);
        return;
    }

    memset(&u, 0, sizeof(struct UserLogin));
    strcpy(u.userID, newID);

    printf("Enter password: ");
    scanf("%29s", u.password);
    clearBuffer();

    printf("Role (0=customer, 1=admin): ");
    u.role = safeReadInt();
    if (u.role != 0 && u.role != 1) u.role = 0;

    if (u.role == 0)
    {
        printf("Link to account number (1-100): ");
        u.linkedAcct = safeReadInt();
    }

    u.locked = 0;

    fseek(fp, emptyPos, SEEK_SET);
    fwrite(&u, sizeof(struct UserLogin), 1, fp);
    fclose(fp);

    printf("User '%s' registered successfully.\n", u.userID);
}

/*
 *   Lets a logged-in user change their own password.
 *   Requires them to enter their old password first for security.
 */
int changePassword(struct UserLogin *currentUser)
{
    char oldPwd[30], newPwd[30];

    printf("Enter current password: ");
    scanf("%29s", oldPwd);
    clearBuffer();

    if (strcmp(oldPwd, currentUser->password) != 0)
    {
        printf("Incorrect current password.\n");
        return 0;
    }

    printf("Enter new password: ");
    scanf("%29s", newPwd);
    clearBuffer();

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
            fclose(fp);
            return 1;
        }
        pos += sizeof(struct UserLogin);
    }
    fclose(fp);
    return 0;
}


/* 7. ACCOUNT OPERATIONS  */

/*
 *   Same as your original — checks account number is 1 to 100.
 *   Used before every fseek to prevent reading outside file bounds.
 */
int validAccount(unsigned int account)
{
    return (account >= 1 && account <= MAX_ACCOUNTS);
}

/*
 *   MAJOR UPGRADE from your original newRecord().
 *   Now collects:
 *     - Full name, address, phone number
 *     - Date of birth → auto-calculates age
 *     - Age < 18 → Minor account (type 3), else user chooses Savings/Current
 *     - 4-digit ATM PIN
 *     - Initial deposit (must meet minimum balance for account type)
 *   Links account to the logged-in userID.
 */

void createAccount(FILE *fPtr, const char *userID)
{
    struct Account client;
    unsigned int account;

    printSeparator();
    printf("         CREATE NEW ACCOUNT\n");
    printSeparator();

    printf("Enter account number (1-%d): ", MAX_ACCOUNTS);
    account = (unsigned int)safeReadInt();

    if (!validAccount(account))
    {
        printf("Invalid account number.\n");
        return;
    }

    /* check for slot*/
    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&client, sizeof(struct Account), 1, fPtr) != 1)
    {
        printf("File read error.\n");
        return;
    }
    if (client.isActive == 1)
    {
        printf("Account %u already exists.\n", account);
        return;
    }

    /* fresh blank record */
    memset(&client, 0, sizeof(struct Account));
    client.acctNum = account;
    strcpy(client.userID, userID);

    /* collect personal details */
    printf("First name: ");
    scanf("%19s", client.firstName);
    clearBuffer();

    printf("Last name: ");
    scanf("%19s", client.lastName);
    clearBuffer();

    printf("Address (no spaces, use _ for spaces): ");
    scanf("%59s", client.address);
    clearBuffer();

    /* phone validation loop */
    do {
        printf("Phone (10 digits): ");
        scanf("%14s", client.phone);
        clearBuffer();
        if (!validatePhone(client.phone))
            printf("Invalid phone. Must be 10 digits.\n");
    } while (!validatePhone(client.phone));

    /* DOB validation loop */
    do {
        printf("Date of birth (DD/MM/YYYY): ");
        scanf("%10s", client.dob);
        clearBuffer();
        if (!validateDOB(client.dob))
            printf("Invalid date. Use DD/MM/YYYY (e.g. 15/08/2000)\n");
    } while (!validateDOB(client.dob));

    /* auto-calculate age from DOB */
    client.age = calculateAge(client.dob);
    printf("Age calculated: %d years\n", client.age);

    /*
     * MINOR ACCOUNT LOGIC:
     * If age < 18, account type is automatically set to Minor (type 3).
     * Minor accounts have lower minimum balance requirements.
     * If age >= 18 (major), user chooses Savings or Current.
     */
    if (client.age < 18)
    {
        client.acctType = 3;
        printf("Minor account will be created (age < 18).\n");
        printf("Initial deposit (min Rs. 100): ");
    }
    else
    {
        printf("Account type:\n");
        printf("  1 - Savings (min balance Rs. %.0f, earns %.1f%% interest/year)\n",
               MIN_SAVINGS, SAVINGS_RATE);
        printf("  2 - Current (min balance Rs. %.0f, service charges apply)\n",
               MIN_CURRENT);
        printf("Choice: ");
        int typeChoice = safeReadInt();
        client.acctType = (typeChoice == 2) ? 2 : 1;
        printf("Initial deposit (min Rs. %.0f): ",
               client.acctType == 1 ? MIN_SAVINGS : MIN_CURRENT);
    }

    client.balance = safeReadDouble();

    /* check minimum opening balance */
    double minRequired = (client.acctType == 1) ? MIN_SAVINGS :
                         (client.acctType == 2) ? MIN_CURRENT : 100.0;

    if (client.balance < minRequired)
    {
        printf("Minimum opening deposit is Rs. %.2f\n", minRequired);
        return;
    }

    /* set ATM PIN */
    int pin = 0;
    do {
        printf("Set 4-digit ATM PIN: ");
        pin = safeReadInt();
        if (pin < 1000 || pin > 9999)
            printf("PIN must be exactly 4 digits.\n");
    } while (pin < 1000 || pin > 9999);
    client.pin = pin;

    client.isActive = 1;

    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&client, sizeof(struct Account), 1, fPtr);

    logTransaction(account, "OPEN", client.balance, client.balance, 0);

    printSeparator();
    printf("Account %u created successfully!\n", account);
    printf("Type: %s | Balance: Rs. %.2f\n",
           client.acctType == 1 ? "Savings" :
           client.acctType == 2 ? "Current" : "Minor",
           client.balance);
    printSeparator();
}

/*
 *   Shows complete profile of one account.
 *   Used by both admin and customer menus.
 */
void displayAccountDetails(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }

    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account %u does not exist.\n", acctNum);
        return;
    }

    printSeparator();
    printf("  ACCOUNT DETAILS - Account #%u\n", c.acctNum);
    printSeparator();
    printf("Name        : %s %s\n", c.firstName, c.lastName);
    printf("Address     : %s\n", c.address);
    printf("Phone       : %s\n", c.phone);
    printf("DOB         : %s  (Age: %d)\n", c.dob, c.age);
    printf("Type        : %s\n",
           c.acctType == 1 ? "Savings" :
           c.acctType == 2 ? "Current" : "Minor");
    printf("Balance     : Rs. %.2f\n", c.balance);
    printf("UserID      : %s\n", c.userID);
    printSeparator();
}

/*
 *   Admin function. Marks account isActive=0 (soft delete).
 *   Logs final balance as outgoing transaction.
 *   WHY SOFT DELETE? We keep the record so transaction history stays valid.
 */
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
        printf("Account does not exist or already closed.\n");
        return;
    }

    printf("Closing account of %s %s. Balance Rs. %.2f will be forfeited.\n",
           c.firstName, c.lastName, c.balance);
    printf("Confirm? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    clearBuffer();
    if (toupper(confirm) != 'Y') { printf("Cancelled.\n"); return; }

    logTransaction(account, "CLOSE", -c.balance, 0.0, 0);
    c.isActive = 0;
    c.balance  = 0.0;

    fseek(fPtr, (account - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, fPtr);

    printf("Account %u closed.\n", account);
}

/*
 *   Shows all active accounts in a table.
 */
void displayAllAccounts(FILE *fPtr)
{
    struct Account c;
    int found = 0;

    rewind(fPtr);
    printSeparator();
    printf("%-6s %-12s %-12s %-10s %-12s\n",
           "Acct", "First Name", "Last Name", "Type", "Balance");
    printSeparator();

    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive)
        {
            found = 1;
            printf("%-6u %-12s %-12s %-10s Rs.%-9.2f\n",
                   c.acctNum, c.firstName, c.lastName,
                   c.acctType == 1 ? "Savings" :
                   c.acctType == 2 ? "Current" : "Minor",
                   c.balance);
        }
    }

    if (!found) printf("No active accounts found.\n");
    printSeparator();
}

// Scans all accounts
 
void searchByName(FILE *fPtr)
{
    char name[30];
    printf("Enter name to search: ");
    scanf("%29s", name);
    clearBuffer();

    struct Account c;
    int found = 0;

    rewind(fPtr);
    printSeparator();

    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive &&
            (strstr(c.firstName, name) || strstr(c.lastName, name)))
        {
            found = 1;
            printf("Acct#%-5u | %s %s | %s | Rs. %.2f\n",
                   c.acctNum, c.firstName, c.lastName,
                   c.acctType == 1 ? "Savings" :
                   c.acctType == 2 ? "Current" : "Minor",
                   c.balance);
        }
    }

    if (!found) printf("No accounts found with name '%s'.\n", name);
    printSeparator();
}


 //Enter min and max balance, see all accounts

void searchByBalanceRange(FILE *fPtr)
{
    double minBal, maxBal;
    printf("Enter minimum balance: ");
    minBal = safeReadDouble();
    printf("Enter maximum balance: ");
    maxBal = safeReadDouble();

    if (minBal > maxBal)
    {
        printf("Minimum cannot be greater than maximum.\n");
        return;
    }

    struct Account c;
    int found = 0;
    rewind(fPtr);
    printSeparator();

    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && c.balance >= minBal && c.balance <= maxBal)
        {
            found = 1;
            printf("Acct#%-5u | %s %s | Rs. %.2f\n",
                   c.acctNum, c.firstName, c.lastName, c.balance);
        }
    }

    if (!found) printf("No accounts in that balance range.\n");
    printSeparator();
}

/*
 * Same bubble sort as your original but works with new Account struct.
 */

void sortAccounts(FILE *fPtr)
{
    struct Account clients[MAX_ACCOUNTS];
    int count = 0;
    struct Account c;

    rewind(fPtr);
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive)
            clients[count++] = c;
    }

    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (clients[j].balance < clients[j+1].balance)
            {
                struct Account tmp = clients[j];
                clients[j] = clients[j+1];
                clients[j+1] = tmp;
            }

    printSeparator();
    printf("  ACCOUNTS SORTED BY BALANCE (HIGH TO LOW)\n");
    printSeparator();
    printf("%-6s %-12s %-12s %-10s %s\n",
           "Acct", "First", "Last", "Type", "Balance");

    for (int i = 0; i < count; i++)
        printf("%-6u %-12s %-12s %-10s Rs. %.2f\n",
               clients[i].acctNum, clients[i].firstName, clients[i].lastName,
               clients[i].acctType == 1 ? "Savings" :
               clients[i].acctType == 2 ? "Current" : "Minor",
               clients[i].balance);

    printSeparator();
}


/*8. TRANSACTION LOGGING  */

/*
 *  It appends one line to transactions.txt.
 *   FORMAT OF EACH LINE:
 *   [DD/MM/YYYY HH:MM:SS] 
 */
void logTransaction(unsigned int acctNum, const char *type,
                    double amount, double newBalance, unsigned int toAcct)
{
    FILE *fp = fopen("transactions.txt", "a");
    if (!fp) { printf("Warning: could not write to transactions.txt\n"); return; }

    char dateTime[25];
    getCurrentDateTime(dateTime, sizeof(dateTime));

    if (toAcct > 0)
    {
        
        fprintf(fp,
            "[%s] | Acct:%-4u | Type:%-12s | Amt:%+10.2f | Bal:%10.2f | To:%u\n",
            dateTime, acctNum, type, amount, newBalance, toAcct);
    }
    else
    {
        fprintf(fp,
            "[%s] | Acct:%-4u | Type:%-12s | Amt:%+10.2f | Bal:%10.2f\n",
            dateTime, acctNum, type, amount, newBalance);
    }

    fclose(fp);
}

/*
 *   Shows ALL transactions for a specific account number.
 */
void viewFullHistory(unsigned int acctNum)
{
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history found.\n"); return; }

    char line[300];
    char search[20];
    sprintf(search, "Acct:%-4u", acctNum);

    int found = 0;
    printSeparator();
    printf("  FULL TRANSACTION HISTORY - Account #%u\n", acctNum);
    printSeparator();

    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, search))
        {
            printf("%s", line);
            found = 1;
        }
    }

    if (!found) printf("No transactions found for account %u.\n", acctNum);
    printSeparator();
    fclose(fp);
}

/*
 *   Shows only the LAST 5 transactions for an account.
 */
void miniStatement(unsigned int acctNum)
{
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history.\n"); return; }

    char lines[1000][300];
    char search[20];
    sprintf(search, "Acct:%-4u", acctNum);

    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), fp) && count < 999)
    {
        if (strstr(lines[count], search))
            count++;
    }
    fclose(fp);

    printSeparator();
    printf("  MINI STATEMENT - Account #%u (Last 5 transactions)\n", acctNum);
    printSeparator();

    int start = (count > 5) ? count - 5 : 0;
    for (int i = start; i < count; i++)
        printf("%s", lines[i]);

    if (count == 0)
        printf("No transactions yet for account %u.\n", acctNum);

    printSeparator();
}

/*
 *  Shows the entire transactions.txt file.
 */
void viewAllTransactions(void)
{
    FILE *fp = fopen("transactions.txt", "r");
    if (!fp) { printf("No transaction history.\n"); return; }

    char line[300];
    printSeparator();
    printf("  ALL TRANSACTION HISTORY\n");
    printSeparator();

    while (fgets(line, sizeof(line), fp))
        printf("%s", line);

    printSeparator();
    fclose(fp);
}


/* 9. DEPOSIT & WITHDRAW  */

/*
 *   Credits money to an account.
 */
void deposit(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }

    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account not found.\n"); return;
    }

    printf("Current balance: Rs. %.2f\n", c.balance);
    printf("Enter deposit amount: Rs. ");
    double amount = safeReadDouble();

    /* validation */
    if (amount <= 0)       { printf("Amount must be positive.\n"); return; }
    if (amount > 100000.0) { printf("Max single deposit: Rs. 1,00,000\n"); return; }

    c.balance += amount;

    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, fPtr);

    logTransaction(acctNum, "DEPOSIT", amount, c.balance, 0);

    printf("Deposited Rs. %.2f | New balance: Rs. %.2f\n", amount, c.balance);
}

/*
 *   Debits money from an account.
 *   VALIDATIONS:
 *   - Amount must be > 0
 *   - Cannot go below minimum balance (Rs.500 savings, Rs.1000 current)
 *   - Minor accounts: withdrawal blocked if under 18 without guardian
 *     (simplified: minor accounts cannot withdraw more than Rs.500/day)
 *   Logs transaction after every successful withdrawal.
 */
void withdraw(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }

    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account not found.\n"); return;
    }

    printf("Current balance: Rs. %.2f\n", c.balance);

    /* determine minimum balance to maintain */
    double minBalance = (c.acctType == 1) ? MIN_SAVINGS :
                        (c.acctType == 2) ? MIN_CURRENT : 100.0;

    printf("Enter withdrawal amount: Rs. ");
    double amount = safeReadDouble();

    if (amount <= 0) { printf("Amount must be positive.\n"); return; }

    /* minor account daily withdrawal limit */
    if (c.acctType == 3 && amount > 500.0)
    {
        printf("Minor accounts: max withdrawal Rs. 500 per transaction.\n");
        return;
    }

    /* check minimum balance */
    if (c.balance - amount < minBalance)
    {
        printf("Insufficient balance. Minimum balance to maintain: Rs. %.2f\n",
               minBalance);
        printf("Max you can withdraw: Rs. %.2f\n", c.balance - minBalance);
        return;
    }

    c.balance -= amount;

    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, fPtr);

    logTransaction(acctNum, "WITHDRAW", -amount, c.balance, 0);

    printf("Withdrawn Rs. %.2f | New balance: Rs. %.2f\n", amount, c.balance);
}


/* 10. FUND TRANSFER */

/*
 * fundTransfer():
 *   Transfers money from one account to another.
 */
void fundTransfer(FILE *fPtr, unsigned int fromAcct)
{
    if (!validAccount(fromAcct)) { printf("Invalid source account.\n"); return; }

    struct Account sender, receiver;

    fseek(fPtr, (fromAcct - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&sender, sizeof(struct Account), 1, fPtr) != 1 || !sender.isActive)
    {
        printf("Source account not found.\n"); return;
    }

    printf("Your balance: Rs. %.2f\n", sender.balance);
    printf("Enter destination account number: ");
    unsigned int toAcct = (unsigned int)safeReadInt();

    if (!validAccount(toAcct)) { printf("Invalid destination account.\n"); return; }
    if (toAcct == fromAcct)   { printf("Cannot transfer to your own account.\n"); return; }

    fseek(fPtr, (toAcct - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&receiver, sizeof(struct Account), 1, fPtr) != 1 || !receiver.isActive)
    {
        printf("Destination account not found.\n"); return;
    }

    printf("Sending to: %s %s\n", receiver.firstName, receiver.lastName);
    printf("Enter transfer amount: Rs. ");
    double amount = safeReadDouble();

    if (amount <= 0)        { printf("Amount must be positive.\n"); return; }
    if (amount > 50000.0)   { printf("Max transfer: Rs. 50,000\n"); return; }

    double minBalance = (sender.acctType == 1) ? MIN_SAVINGS :
                        (sender.acctType == 2) ? MIN_CURRENT : 100.0;

    if (sender.balance - amount < minBalance)
    {
        printf("Insufficient funds. Max transferable: Rs. %.2f\n",
               sender.balance - minBalance);
        return;
    }

    sender.balance   -= amount;
    receiver.balance += amount;

    /* write sender back */
    fseek(fPtr, (fromAcct - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&sender, sizeof(struct Account), 1, fPtr);

    /* write receiver back */
    fseek(fPtr, (toAcct - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&receiver, sizeof(struct Account), 1, fPtr);

    /* log both sides of the transfer */
    logTransaction(fromAcct, "TRANSFER_OUT", -amount, sender.balance, toAcct);
    logTransaction(toAcct,   "TRANSFER_IN",  amount,  receiver.balance, fromAcct);

    printf("Transfer successful!\n");
    printf("Sent Rs. %.2f to Account #%u (%s %s)\n",
           amount, toAcct, receiver.firstName, receiver.lastName);
    printf("Your new balance: Rs. %.2f\n", sender.balance);
}


/* 11. INTEREST & CHARGES  */

/*
 * applyInterest():
 *   Admin function. Applied monthly (simulate by calling from admin menu).
 *   For every ACTIVE SAVINGS account:
 *     monthly interest = balance * (SAVINGS_RATE / 100) / 12
 *   Current and Minor accounts do NOT earn interest.
 *   Logs as type "INTEREST".
 */
void applyInterest(FILE *fPtr)
{
    struct Account c;
    int count = 0;

    rewind(fPtr);

    printf("\nApplying monthly interest (%.1f%% annual) to savings accounts...\n",
           SAVINGS_RATE);

    long pos = 0;
    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && c.acctType == 1)   /* only savings */
        {
            double interest = c.balance * (SAVINGS_RATE / 100.0) / 12.0;
            c.balance += interest;

            fseek(fPtr, pos, SEEK_SET);
            fwrite(&c, sizeof(struct Account), 1, fPtr);
            fseek(fPtr, pos + sizeof(struct Account), SEEK_SET);

            logTransaction(c.acctNum, "INTEREST", interest, c.balance, 0);

            printf("Acct#%u | Interest: Rs. %.2f | New balance: Rs. %.2f\n",
                   c.acctNum, interest, c.balance);
            count++;
        }
        pos += sizeof(struct Account);
    }

    printf("Interest applied to %d savings accounts.\n", count);
}

/*
 *   Applied monthly to CURRENT accounts.
 *   Flat service charge of Rs. 50 per month.
 *   If balance is at minimum, no charge applied (avoids going negative).
 */
void applyServiceCharge(FILE *fPtr)
{
    struct Account c;
    double charge = 50.0;
    int count = 0;

    rewind(fPtr);

    printf("\nApplying service charge (Rs. %.2f) to current accounts...\n", charge);

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
                printf("Acct#%u | Charge: Rs. %.2f | Balance: Rs. %.2f\n",
                       c.acctNum, charge, c.balance);
                count++;
            }
            else
            {
                printf("Acct#%u | Skipped (balance too low)\n", c.acctNum);
            }
        }
        pos += sizeof(struct Account);
    }

    printf("Service charge applied to %d current accounts.\n", count);
}


/* 12. FIXED DEPOSIT ─*/

/*
 *   Deducts principal from the account balance and creates an FD record.
 *   Calculates maturity date by adding tenure months to today.
 *   Calculates maturity amount using simple interest:
 *     maturity = principal + (principal * rate * years / 100)
 */
void createFD(FILE *acctFile, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }

    struct Account c;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, acctFile) != 1 || !c.isActive)
    {
        printf("Account not found.\n"); return;
    }

    printf("Available balance: Rs. %.2f\n", c.balance);
    printf("Enter FD amount: Rs. ");
    double principal = safeReadDouble();

    if (principal <= 0) { printf("Amount must be positive.\n"); return; }
    if (principal > c.balance)
    {
        printf("Insufficient balance for FD.\n"); return;
    }

    printf("Enter tenure (months, 1-120): ");
    int months = safeReadInt();
    if (months < 1 || months > 120) { printf("Invalid tenure.\n"); return; }

    /* calculate maturity */
    double years   = months / 12.0;
    double interest = principal * FD_RATE * years / 100.0;
    double maturity = principal + interest;

    char startDate[11], maturityDate[11];
    getCurrentDate(startDate, sizeof(startDate));
    addMonthsToDate(startDate, months, maturityDate);

    /* find empty FD slot */
    FILE *fdFile = fopen("fd.dat", "rb+");
    if (!fdFile) { printf("Cannot open fd.dat\n"); return; }

    struct FixedDeposit fd;
    long emptyPos = -1;
    int newFDID = 1;
    long pos = 0;

    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (!fd.isActive && emptyPos == -1)
            emptyPos = pos;
        else if (fd.isActive && fd.fdID >= newFDID)
            newFDID = fd.fdID + 1;
        pos += sizeof(struct FixedDeposit);
    }

    if (emptyPos == -1) { printf("FD storage full.\n"); fclose(fdFile); return; }

    /* build FD record */
    memset(&fd, 0, sizeof(struct FixedDeposit));
    fd.fdID          = newFDID;
    fd.acctNum       = acctNum;
    fd.principal     = principal;
    fd.tenureMonths  = months;
    strcpy(fd.startDate, startDate);
    strcpy(fd.maturityDate, maturityDate);
    fd.maturityAmount = maturity;
    fd.isActive       = 1;

    fseek(fdFile, emptyPos, SEEK_SET);
    fwrite(&fd, sizeof(struct FixedDeposit), 1, fdFile);
    fclose(fdFile);

    /* deduct from account balance */
    c.balance -= principal;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, acctFile);

    logTransaction(acctNum, "FD_OPEN", -principal, c.balance, 0);

    printSeparator();
    printf("Fixed Deposit Created!\n");
    printf("FD ID       : %d\n", fd.fdID);
    printf("Principal   : Rs. %.2f\n", principal);
    printf("Tenure      : %d months\n", months);
    printf("Start Date  : %s\n", startDate);
    printf("Maturity    : %s\n", maturityDate);
    printf("At Maturity : Rs. %.2f (%.1f%% p.a. simple interest)\n",
           maturity, FD_RATE);
    printf("Remaining balance: Rs. %.2f\n", c.balance);
    printSeparator();
}

/*
 *   Shows all active FDs belonging to a specific account.
 */
void viewMyFDs(unsigned int acctNum)
{
    FILE *fdFile = fopen("fd.dat", "rb");
    if (!fdFile) { printf("No FD data found.\n"); return; }

    struct FixedDeposit fd;
    int found = 0;

    printSeparator();
    printf("  FIXED DEPOSITS - Account #%u\n", acctNum);
    printSeparator();

    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (fd.isActive && fd.acctNum == (int)acctNum)
        {
            found = 1;
            printf("FD#%d | Principal: Rs.%.2f | Tenure: %d months\n",
                   fd.fdID, fd.principal, fd.tenureMonths);
            printf("      | Start: %s | Maturity: %s | At maturity: Rs.%.2f\n",
                   fd.startDate, fd.maturityDate, fd.maturityAmount);
        }
    }

    if (!found) printf("No active FDs for account %u.\n", acctNum);
    printSeparator();
    fclose(fdFile);
}

/*
 *   Closes (breaks) an FD.
 *   IF before maturity date: principal credited back, penalty deducted.
 */
void closeFD(FILE *acctFile, unsigned int acctNum)
{
    FILE *fdFile = fopen("fd.dat", "rb+");
    if (!fdFile) { printf("No FD data.\n"); return; }

    printf("Enter FD ID to close: ");
    int fdID = safeReadInt();

    struct FixedDeposit fd;
    long pos = 0;
    int found = 0;

    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (fd.isActive && fd.fdID == fdID && fd.acctNum == (int)acctNum)
        {
            found = 1; break;
        }
        pos += sizeof(struct FixedDeposit);
    }

    if (!found)
    {
        printf("FD #%d not found for account %u.\n", fdID, acctNum);
        fclose(fdFile); return;
    }

    printf("FD #%d | Principal: Rs.%.2f | Maturity: %s | At maturity: Rs.%.2f\n",
           fd.fdID, fd.principal, fd.maturityDate, fd.maturityAmount);

    printf("Is this FD mature? (y=mature, n=premature closure): ");
    char ans;
    scanf(" %c", &ans);
    clearBuffer();

    double credited;
    if (toupper(ans) == 'Y')
    {
        /* mature: full maturity amount */
        credited = fd.maturityAmount;
        printf("Crediting full maturity amount: Rs. %.2f\n", credited);
    }
    else
    {
        /* premature: principal - penalty% */
        double penalty = fd.principal * FD_PENALTY / 100.0;
        credited = fd.principal - penalty;
        printf("Premature closure. Penalty: Rs. %.2f\n", penalty);
        printf("Credited: Rs. %.2f\n", credited);
    }

    /* credit to account */
    struct Account c;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fread(&c, sizeof(struct Account), 1, acctFile);
    c.balance += credited;
    fseek(acctFile, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    fwrite(&c, sizeof(struct Account), 1, acctFile);

    /* mark FD inactive */
    fd.isActive = 0;
    fseek(fdFile, pos, SEEK_SET);
    fwrite(&fd, sizeof(struct FixedDeposit), 1, fdFile);
    fclose(fdFile);

    logTransaction(acctNum, "FD_CLOSE", credited, c.balance, 0);

    printf("FD closed. New account balance: Rs. %.2f\n", c.balance);
}

/*
 *   Admin function. Shows every active FD across all accounts.
 */
void viewAllFDs(void)
{
    FILE *fdFile = fopen("fd.dat", "rb");
    if (!fdFile) { printf("No FD data.\n"); return; }

    struct FixedDeposit fd;
    int found = 0;

    printSeparator();
    printf("  ALL ACTIVE FIXED DEPOSITS\n");
    printSeparator();

    while (fread(&fd, sizeof(struct FixedDeposit), 1, fdFile) == 1)
    {
        if (fd.isActive)
        {
            found = 1;
            printf("FD#%-3d | Acct#%-4d | Rs.%.2f | %d months | "
                   "Matures: %s | At maturity: Rs.%.2f\n",
                   fd.fdID, fd.acctNum, fd.principal,
                   fd.tenureMonths, fd.maturityDate, fd.maturityAmount);
        }
    }

    if (!found) printf("No active FDs.\n");
    printSeparator();
    fclose(fdFile);
}


/*  13. ATM SIMULATION  */

/*
 *   Simulates an ATM machine for a specific account.
 *
 *   FLOW:
 *   1. Ask for 4-digit PIN — up to MAX_ATTEMPTS attempts
 *   2. If PIN correct → show ATM options menu
 *   3. Options: Withdraw / Deposit / Balance / Mini Statement / Exit
 *
 *   ATM WITHDRAW RULES:
 *   - Must be multiple of 100 (ATM dispenses in 100s)
 *   - Max single withdrawal: Rs. 10,000
 *   - Minimum balance must be maintained

 */
void atmMenu(FILE *fPtr, unsigned int acctNum)
{
    if (!validAccount(acctNum)) { printf("Invalid account.\n"); return; }

    struct Account c;
    fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
    if (fread(&c, sizeof(struct Account), 1, fPtr) != 1 || !c.isActive)
    {
        printf("Account not found.\n"); return;
    }

    /* PIN verification */
    int attempts = 0;
    int authenticated = 0;

    printSeparator();
    printf("  WELCOME TO ATM\n");
    printSeparator();

    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter 4-digit PIN: ");
        int enteredPIN = safeReadInt();

        if (enteredPIN == c.pin)
        {
            authenticated = 1;
            break;
        }
        attempts++;
        printf("Wrong PIN. Attempts left: %d\n", MAX_ATTEMPTS - attempts);
    }

    if (!authenticated)
    {
        printf("Card blocked due to too many wrong PIN attempts.\n");
        return;
    }

    /* ATM operations menu */
    int atmChoice;
    do {
        printSeparator();
        printf("  ATM MENU - Acct#%u | %s %s\n",
               c.acctNum, c.firstName, c.lastName);
        printSeparator();
        printf("1 - Withdraw\n");
        printf("2 - Deposit\n");
        printf("3 - Check Balance\n");
        printf("4 - Mini Statement\n");
        printf("5 - Exit ATM\n");
        printf("Choice: ");
        atmChoice = safeReadInt();

        /* reload account data before each operation (in case another
           operation changed balance) */
        fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
        fread(&c, sizeof(struct Account), 1, fPtr);

        switch (atmChoice)
        {
        case 1:
        {
            printf("Enter amount (multiples of 100, max Rs.10000): Rs. ");
            double amt = safeReadDouble();

            if ((int)amt % 100 != 0)
            { printf("ATM dispenses in multiples of Rs. 100 only.\n"); break; }

            if (amt > 10000.0)
            { printf("ATM single withdrawal limit: Rs. 10,000\n"); break; }

            double minBal = (c.acctType == 1) ? MIN_SAVINGS :
                            (c.acctType == 2) ? MIN_CURRENT : 100.0;

            if (c.balance - amt < minBal)
            {
                printf("Insufficient balance. Max: Rs. %.2f\n",
                       c.balance - minBal);
                break;
            }

            c.balance -= amt;
            fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
            fwrite(&c, sizeof(struct Account), 1, fPtr);

            logTransaction(acctNum, "ATM_WITHDRAW", -amt, c.balance, 0);

            printf("Please collect Rs. %.0f\n", amt);
            printf("Remaining balance: Rs. %.2f\n", c.balance);
            break;
        }
        case 2:
        {
            printf("Insert amount to deposit: Rs. ");
            double amt = safeReadDouble();
            if (amt <= 0) { printf("Invalid amount.\n"); break; }

            c.balance += amt;
            fseek(fPtr, (acctNum - 1) * sizeof(struct Account), SEEK_SET);
            fwrite(&c, sizeof(struct Account), 1, fPtr);

            logTransaction(acctNum, "ATM_DEPOSIT", amt, c.balance, 0);
            printf("Deposited Rs. %.2f | Balance: Rs. %.2f\n", amt, c.balance);
            break;
        }
        case 3:
            printf("Available Balance: Rs. %.2f\n", c.balance);
            break;

        case 4:
            miniStatement(acctNum);
            break;

        case 5:
            printf("Thank you for using ATM. Goodbye!\n");
            break;

        default:
            printf("Invalid choice.\n");
        }

    } while (atmChoice != 5);
}


/* 14. ACCOUNT SUMMARY & REPORTS */

/*
 *   - Total accounts (broken down by type)
 *   - Total balance across all accounts
 *   - Average balance
 *   Improved from your original: now shows per-type breakdown.
 */
void accountSummary(FILE *fPtr)
{
    struct Account c;
    int total = 0, savings = 0, current = 0, minor = 0;
    double totalBal = 0, savBal = 0, curBal = 0, minBal2 = 0;

    rewind(fPtr);

    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive)
        {
            total++;
            totalBal += c.balance;
            if (c.acctType == 1) { savings++; savBal += c.balance; }
            if (c.acctType == 2) { current++; curBal += c.balance; }
            if (c.acctType == 3) { minor++;   minBal2 += c.balance; }
        }
    }

    printSeparator();
    printf("  ACCOUNT SUMMARY REPORT\n");
    printSeparator();
    printf("Total Active Accounts : %d\n", total);
    printf("  Savings accounts    : %d  (Total: Rs. %.2f)\n", savings, savBal);
    printf("  Current accounts    : %d  (Total: Rs. %.2f)\n", current, curBal);
    printf("  Minor accounts      : %d  (Total: Rs. %.2f)\n", minor, minBal2);
    printf("Total Balance (Bank)  : Rs. %.2f\n", totalBal);
    printf("Average Balance       : Rs. %.2f\n",
           total > 0 ? totalBal / total : 0.0);
    printSeparator();
}

/*
 *   Same as your original textFile() — exports all active accounts
 *   to accounts.txt in a human-readable table format.
 */
void exportToTextFile(FILE *fPtr)
{
    FILE *wp = fopen("accounts.txt", "w");
    if (!wp) { printf("Cannot create accounts.txt\n"); return; }

    struct Account c;
    rewind(fPtr);

    fprintf(wp, "%-6s %-12s %-12s %-10s %-15s\n",
            "Acct", "First", "Last", "Type", "Balance");
    fprintf(wp, "------------------------------------------------------------\n");

    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive)
        {
            fprintf(wp, "%-6u %-12s %-12s %-10s Rs. %-12.2f\n",
                    c.acctNum, c.firstName, c.lastName,
                    c.acctType == 1 ? "Savings" :
                    c.acctType == 2 ? "Current" : "Minor",
                    c.balance);
        }
    }

    fclose(wp);
    printf("accounts.txt created successfully.\n");
}


/* 15. BACKUP & RESTORE */

/*
 *   Copies all three binary data files to backup versions:
 *     credit.dat   → backup_credit.dat
 *     fd.dat       → backup_fd.dat
 *     users.dat    → backup_users.dat
 *   Also backs up transactions.txt → backup_transactions.txt
 */
void backupData(void)
{
    const char *files[] = {
        "credit.dat", "fd.dat", "users.dat", "transactions.txt"
    };
    const char *backups[] = {
        "backup_credit.dat", "backup_fd.dat",
        "backup_users.dat",  "backup_transactions.txt"
    };
    int n = 4;

    printf("\nBacking up data files...\n");

    for (int i = 0; i < n; i++)
    {
        FILE *src = fopen(files[i], "rb");
        if (!src) { printf("Skipping %s (not found)\n", files[i]); continue; }

        FILE *dst = fopen(backups[i], "wb");
        if (!dst) { printf("Cannot create %s\n", backups[i]); fclose(src); continue; }

        char buf[512];
        size_t bytesRead;
        while ((bytesRead = fread(buf, 1, sizeof(buf), src)) > 0)
            fwrite(buf, 1, bytesRead, dst);

        fclose(src);
        fclose(dst);
        printf("Backed up: %s → %s\n", files[i], backups[i]);
    }
    printf("Backup complete.\n");
}

/*
 *   Reverse of backup — copies backup files back to live files.
 *   WARNING: overwrites current data!
 */
void restoreData(void)
{
    printf("WARNING: This will overwrite all current data with backup!\n");
    printf("Continue? (y/n): ");
    char ans;
    scanf(" %c", &ans);
    clearBuffer();
    if (toupper(ans) != 'Y') { printf("Restore cancelled.\n"); return; }

    const char *backups[] = {
        "backup_credit.dat", "backup_fd.dat",
        "backup_users.dat",  "backup_transactions.txt"
    };
    const char *files[] = {
        "credit.dat", "fd.dat", "users.dat", "transactions.txt"
    };
    int n = 4;

    for (int i = 0; i < n; i++)
    {
        FILE *src = fopen(backups[i], "rb");
        if (!src) { printf("Backup not found: %s\n", backups[i]); continue; }

        FILE *dst = fopen(files[i], "wb");
        if (!dst) { printf("Cannot write: %s\n", files[i]); fclose(src); continue; }

        char buf[512];
        size_t bytesRead;
        while ((bytesRead = fread(buf, 1, sizeof(buf), src)) > 0)
            fwrite(buf, 1, bytesRead, dst);

        fclose(src);
        fclose(dst);
        printf("Restored: %s → %s\n", backups[i], files[i]);
    }
    printf("Restore complete. Restart program for changes to take effect.\n");
}


/*16. ADMIN MENU */

/*
 *   Full control panel for admin users.
 *   Admin can access all accounts, run reports, apply interest, manage users.
 */
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
        printf("Choice: ");
        choice = safeReadInt();

        unsigned int acct;

        switch (choice)
        {
        case 1:  createAccount(fPtr, admin->userID); break;
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
            printf("Enter account number: ");
            acct = (unsigned int)safeReadInt();
            displayAccountDetails(fPtr, acct);
            break;
        case 0:
            printf("Logging out...\n");
            break;
        default:
            printf("Invalid choice.\n");
        }

        if (choice != 0) pressEnterToContinue();

    } while (choice != 0);
}


/* 17. CUSTOMER MENU */

/*
 *   Limited menu for regular customers.
 *   Customer can ONLY access their own account (linked via userID).
 *   They cannot see other customers' data.
 */
void customerMenu(FILE *fPtr, struct UserLogin *user)
{
    unsigned int myAcct = 0;
    struct Account c;
    rewind(fPtr);

    while (fread(&c, sizeof(struct Account), 1, fPtr) == 1)
    {
        if (c.isActive && strcmp(c.userID, user->userID) == 0)
        {
            myAcct = c.acctNum;
            break;
        }
    }

    if (myAcct == 0)
    {
        printf("No account linked to your username '%s'.\n", user->userID);
        printf("Please contact admin to link your account.\n");
        return;
    }

    int choice;

    do {
        /* reload account data for display */
        fseek(fPtr, (myAcct - 1) * sizeof(struct Account), SEEK_SET);
        fread(&c, sizeof(struct Account), 1, fPtr);

        printSeparator();
        printf("  CUSTOMER MENU | %s %s | Acct#%u | Rs. %.2f\n",
               c.firstName, c.lastName, myAcct, c.balance);
        printSeparator();
        printf("1 - View my account details\n");
        printf("2 - Deposit\n");
        printf("3 - Withdraw\n");
        printf("4 - Fund transfer\n");
        printf("5 - ATM simulation\n");
        printf("6 - View full transaction history\n");
        printf("7 - Mini statement (last 5)\n");
        printf("8 - Create Fixed Deposit\n");
        printf("9 - View my Fixed Deposits\n");
        printf("10 - Close a Fixed Deposit\n");
        printf("11 - Change password\n");
        printf("0  - Logout\n");
        printf("Choice: ");
        choice = safeReadInt();

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


/* 18. MAIN */

/*
 *   Entry point. Initializes files, runs login, routes to admin or customer menu.
 */
int main(void)
{
    /* create files if missing */
    initializeAllFiles();

    /* open main account file */
    FILE *cfPtr = fopen("credit.dat", "rb+");
    if (!cfPtr)
    {
        printf("FATAL: Could not open credit.dat\n");
        return 1;
    }

    /*  login — up to 3 attempts */
    struct UserLogin currentUser;
    if (!loginSystem(&currentUser))
    {
        printf("Login failed. Exiting.\n");
        fclose(cfPtr);
        return 1;
    }

    /*  route by role */
    if (currentUser.role == 1)
        adminMenu(cfPtr, &currentUser);
    else
        customerMenu(cfPtr, &currentUser);

    fclose(cfPtr);
    printf("\nThank you for using Banking Management System. Goodbye!\n");
    return 0;
}
