# Banking Management System in C

A complete terminal-based banking management system built in C using binary file handling for persistent data storage. Simulates real-world banking operations including account management, fund transfers, fixed deposits, ATM simulation, and a role-based login system.

---

## Features

### Admin Features
| Option | Feature |
|--------|---------|
| 1 | Create new bank account (Savings / Current / Minor) |
| 2 | Delete / close an account |
| 3 | View all active accounts |
| 4 | Search accounts by customer name |
| 5 | Search accounts by balance range |
| 6 | Sort all accounts by balance (high to low) |
| 7 | Account summary report (per type, totals, average) |
| 8 | Export all accounts to `accounts.txt` |
| 9 | View all transactions (full audit log) |
| 10 | Apply monthly interest to all savings accounts |
| 11 | Apply monthly service charge to current accounts |
| 12 | View all fixed deposits |
| 13 | Register a new user (admin or customer) |
| 14 | Backup all data files |
| 15 | Restore from backup |
| 16 | Change password |
| 17 | View specific account details |

### Customer Features
| Option | Feature |
|--------|---------|
| 1 | View my account details |
| 2 | Deposit money |
| 3 | Withdraw money |
| 4 | Transfer money to another account |
| 5 | ATM simulation (PIN + Withdraw / Deposit / Balance / Mini Statement) |
| 6 | View full transaction history |
| 7 | Mini statement (last 5 transactions) |
| 8 | Create Fixed Deposit |
| 9 | View my Fixed Deposits |
| 10 | Close a Fixed Deposit |
| 11 | Change password |

---

## Account Types

| Type | Min Balance | Interest | Notes |
|------|------------|----------|-------|
| Savings | Rs. 500 | 3% p.a. (monthly) | Standard savings account |
| Current | Rs. 1,000 | None | Rs. 50/month service charge |
| Minor | Rs. 100 | None | Auto-assigned if age < 18, max Rs. 500/withdrawal |

Age is automatically calculated from Date of Birth. If age < 18, a Minor account is created automatically.

---

## Data Files

| File | Type | Purpose |
|------|------|---------|
| `credit.dat` | Binary | Stores 100 Account records |
| `fd.dat` | Binary | Stores 50 Fixed Deposit records |
| `users.dat` | Binary | Stores 50 UserLogin records |
| `transactions.txt` | Text | Appended transaction log with date and time |
| `accounts.txt` | Text | Human-readable export of all accounts |
| `backup_*.dat` | Binary | Backup copies of all data files |

---

## How to Compile and Run

### Linux / Mac
```bash
gcc -o bank bank.c
./bank
```

### Windows (CMD / PowerShell)
```bash
gcc -o bank.exe bank.c
bank.exe
```

### Windows — GCC 15 Error Fix
If you get a `#pragma pack` error with GCC 15.x on MinGW:
```bash
gcc -D__USE_MINGW_ANSI_STDIO=1 -o bank.exe bank.c
```
Recommended: Use GCC 13.x via [MSYS2](https://www.msys2.org/) for stable Windows compilation.

---

## First Run — Default Login

On first run, all data files are created automatically.

```
Username: admin
Password: admin123
```

> Change the admin password immediately after first login using option 16.

---

## How to Add a Customer

Follow this exact sequence so the customer can log in correctly:

1. Log in as admin
2. **Option 1** — Create account (e.g. account number `5`)
3. **Option 13** — Register user
   - Username: `john`
   - Password: `pass123`
   - Role: `0` (customer)
   - Link to account: `5`
4. Logout (option 0)
5. Log in as `john` / `pass123` → Customer menu opens for account #5

---

## Transaction Log Format

Every operation writes one line to `transactions.txt`:

```
[DD/MM/YYYY HH:MM:SS] | Acct:5    | DEPOSIT      | Amt:  +2000.00 | Bal:   7000.00
[DD/MM/YYYY HH:MM:SS] | Acct:5    | TRANSFER_OUT | Amt:  -1500.00 | Bal:   5500.00 | To:8
[DD/MM/YYYY HH:MM:SS] | Acct:8    | TRANSFER_IN  | Amt:  +1500.00 | Bal:   3500.00 | To:5
```

Transfer operations write **two entries** — one for sender (TRANSFER_OUT) and one for receiver (TRANSFER_IN) — so both parties see it in their own history.

---

## Fixed Deposit Rules

- Minimum tenure: 1 month, Maximum: 120 months
- Interest rate: **6.5% p.a.** (simple interest)
- Maturity amount = Principal + (Principal × Rate × Years / 100)
- **Premature closure**: 1.5% penalty deducted from principal
- The FD amount is deducted from the account balance when created and credited back on closure

---

## Security Features

- Login locked after **3 wrong password attempts**
- ATM PIN blocked after **3 wrong PIN attempts**
- Minimum balance enforced — account cannot go below threshold
- Minor accounts restricted to Rs. 500 per withdrawal
- Fund transfer capped at Rs. 50,000 per transaction
- Deposit capped at Rs. 1,00,000 per transaction

---

## Key C Concepts Used

| Concept | Where Used |
|---------|-----------|
| `struct` | Account, FixedDeposit, UserLogin |
| Binary file I/O (`fread`, `fwrite`) | All data storage |
| Random access (`fseek`) | Jump directly to any account record |
| Pointers (`FILE*`, `struct*`) | Pass file handles and structs to functions |
| `strcmp`, `strcpy`, `strstr` | Login comparison, name search |
| `time.h` — `time()`, `localtime()`, `strftime()` | Transaction timestamps |
| `memset` | Zero-initialise structs before use |
| `isdigit`, `toupper` — `ctype.h` | Phone validation, y/n confirmation |
| Bubble sort | Sort accounts by balance |
| `sscanf` | Parse DOB string into day/month/year |
| Append mode `"a"` | Transaction log — never overwrites |
| `do-while` with validation | Input loops until valid data entered |

---

## Project Structure

```
banking-management-system/
│
├── bank.c               ← Main source file (1,196 lines)
├── credit.dat           ← Created on first run (account data)
├── fd.dat               ← Created on first run (FD data)
├── users.dat            ← Created on first run (login data)
├── transactions.txt     ← Created on first run (transaction log)
├── accounts.txt         ← Created when admin exports
└── README.md
```

---

## Git Commands to Push This Project

```bash
# Step 1: Initialise git in your project folder
git init

# Step 2: Connect to your GitHub repo
git remote add origin https://github.com/yourusername/banking-management-system.git

# Step 3: First commit — original code
git add bank.c
git commit -m "Initial commit: Banking Management System in C (1196 lines)"

# Step 4: Push to GitHub
git push -u origin main
```

---

## What I Learned Building This

- How binary file handling works in C — `fread`/`fwrite` with structs, `fseek` for random access
- Why `memset` is important for initialising structs before writing to files
- How to build a role-based login system using file-stored credentials
- How input buffers cause bugs and how `clearBuffer()` fixes them
- Practical use of `strcmp`, `strstr`, `sscanf` for string operations
- How to implement a transaction audit trail using append-mode file writing
- How bubble sort works on an array of structs
- Real-world validation: phone numbers, dates, minimum balance rules

---

## Author

Built as a C programming mini project demonstrating file handling, structs, and real-world system design.
