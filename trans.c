#include <stdio.h>
#include <stdlib.h>

#define MAX_ACCOUNTS 100

// Structure definition
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// Function prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void displayAll(FILE *fPtr);
void searchAccount(FILE *fPtr);
int validAccount(unsigned int account);

// Main
int main()
{
    FILE *cfPtr;
    unsigned int choice;

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("File could not be opened.\n");
        exit(1);
    }

    while ((choice = enterChoice()) != 7)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 5:
            displayAll(cfPtr);
            break;
        case 6:
            searchAccount(cfPtr);
            break;
        default:
            printf("Invalid choice.\n");
        }
    }

    fclose(cfPtr);
    printf("Program ended.\n");
    return 0;
}

// Validate account number
int validAccount(unsigned int account)
{
    return (account >= 1 && account <= MAX_ACCOUNTS);
}

// Create formatted text file
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client;

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        printf("Could not create text file.\n");
        return;
    }

    rewind(readPtr);

    fprintf(writePtr, "%-6s%-16s%-11s%10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);
    printf("accounts.txt created successfully.\n");
}

// Update account with negative balance prevention
void updateRecord(FILE *fPtr)
{
    struct clientData client;
    unsigned int account;
    double transaction;

    printf("Enter account number to update (1 - 100): ");
    scanf("%u", &account);

    if (!validAccount(account))
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account does not exist.\n");
        return;
    }

    printf("Current Balance: %.2f\n", client.balance);
    printf("Enter charge (+) or payment (-): ");
    scanf("%lf", &transaction);

    if (client.balance + transaction < 0)
    {
        printf("Transaction denied. Balance cannot go negative.\n");
        return;
    }

    client.balance += transaction;

    fseek(fPtr, -((long)sizeof(struct clientData)), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Balance updated successfully.\n");
}

// Add new account
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int account;

    printf("Enter new account number (1 - 100): ");
    scanf("%u", &account);

    if (!validAccount(account))
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0)
    {
        printf("Account already exists.\n");
        return;
    }

    printf("Enter lastname firstname balance: ");
    scanf("%14s%9s%lf",
          client.lastName,
          client.firstName,
          &client.balance);

    if (client.balance < 0)
    {
        printf("Initial balance cannot be negative.\n");
        return;
    }

    client.acctNum = account;

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Account created successfully.\n");
}

// Delete account
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0.0};
    unsigned int account;

    printf("Enter account number to delete (1 - 100): ");
    scanf("%u", &account);

    if (!validAccount(account))
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account does not exist.\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);

    printf("Account deleted successfully.\n");
}

// Display all accounts
void displayAll(FILE *fPtr)
{
    struct clientData client;

    rewind(fPtr);

    printf("\n%-6s%-16s%-11s%10s\n",
           "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName,
                   client.balance);
        }
    }
}

// Search account
void searchAccount(FILE *fPtr)
{
    struct clientData client;
    unsigned int account;

    printf("Enter account number to search (1 - 100): ");
    scanf("%u", &account);

    if (!validAccount(account))
    {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account does not exist.\n");
        return;
    }

    printf("\nAccount Details:\n");
    printf("Account Number : %u\n", client.acctNum);
    printf("Last Name      : %s\n", client.lastName);
    printf("First Name     : %s\n", client.firstName);
    printf("Balance        : %.2f\n", client.balance);
}

// Menu
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\nEnter your choice\n"
           "1 - store formatted text file (accounts.txt)\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - display all accounts\n"
           "6 - search an account\n"
           "7 - end program\n? ");

    scanf("%u", &choice);
    return choice;
}
