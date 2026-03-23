# 📘 Banking Management System in C

---

## 🏦 Project Title
**Full Banking Management System in C**

---

## 📌 Overview

This project is a **complete banking system simulation** written in C. It extends a basic file-based account system into a **real-world banking application** with features like:

- Account management (Savings / Current / Minor)
- Secure login system (Admin & Customer)
- ATM simulation
- Transaction logging
- Fixed deposits (FD)
- Interest & service charges
- Backup & restore system

The system uses **binary files (`.dat`) and text logs** to persist data across executions.

---

## ⚙️ Technologies Used

- Language: **C**
- Concepts:
  - File Handling (`rb`, `wb`, `rb+`)
  - Structures
  - Pointers
  - Modular programming
  - Input validation
  - Real-world banking logic

---

## 📂 Files Used in System

| File Name | Purpose |
|----------|--------|
| `credit.dat` | Stores all bank account records |
| `users.dat` | Stores login credentials |
| `fd.dat` | Stores fixed deposit records |
| `transactions.txt` | Logs all transactions |
| `accounts.txt` | Exported readable account data |
| `backup_*.dat` | Backup files |

---

## 🚀 Features Implemented

### 🔐 1. Login System
- Username + Password authentication
- Roles:
  - Admin
  - Customer
- Account locking after 3 failed attempts

---

### 👤 2. Account Management
- Create account with:
  - Name, address, phone
  - DOB → auto age calculation
- Account types:
  - Savings
  - Current
  - Minor (auto if age < 18)
- Minimum balance enforcement

---

### 💰 3. Transactions
- Deposit (max ₹1,00,000 per transaction)
- Withdraw with rules:
  - Minimum balance maintained
  - Minor withdrawal limit
- Fund transfer between accounts

---

### 🧾 4. Transaction Logging
- Every transaction stored in:
transaction.txt
- Includes:
- Date & time
- Type
- Amount
- Balance

---

### 🏧 5. ATM Simulation
- PIN authentication
- Features:
- Withdraw (₹100 multiples)
- Deposit
- Balance check
- Mini statement

---

### 📊 6. Reports & Search
- Search by name
- Search by balance range
- Sort accounts by balance
- Account summary report

---

### 📈 7. Interest & Charges
- Savings:
- Monthly interest applied
- Current:
- Monthly service charge

---

### 💼 8. Fixed Deposit System
- Create FD
- View FD
- Close FD:
- Mature → full amount
- Premature → penalty applied

---

### 💾 9. Backup & Restore
- Backup all data files
- Restore from backup
- Prevents data loss

---

## 🧠 Structure Explanation

### 🧱 1. `struct Account`
Stores customer and account details:
- Account number
- Name, address, phone
- DOB and age
- Balance
- Account type
- PIN

---

### 🏦 2. `struct FixedDeposit`
- FD ID
- Principal amount
- Tenure
- Maturity date
- Maturity amount

---

### 🔑 3. `struct UserLogin`
- Username
- Password
- Role (Admin/Customer)
- Linked account
- Lock status

---

## 🔧 Function Breakdown (Important)

### 🛠 Utility Functions

#### `safeReadInt()`
- Safely reads integer input
- Prevents invalid input crash

#### `safeReadDouble()`
- Reads double safely
- Returns sentinel on error

#### `calculateAge(dob)`
- Converts DOB → age using system date

#### `validateDOB()`
- Ensures correct date format

#### `validatePhone()`
- Ensures 10-digit number

---

### 🔐 Login Functions

#### `loginSystem()`
- Handles authentication
- Limits attempts

#### `registerUser()`
- Creates new user

#### `lockUser()`
- Locks account after failed attempts

---

### 👤 Account Functions

#### `createAccount()`
- Creates new account with validations

#### `displayAccountDetails()`
- Shows full account info

#### `deleteAccount()`
- Soft delete (keeps history)

#### `validAccount()`
- Ensures account number range

---

### 💰 Transaction Functions

#### `deposit()`
- Adds money

#### `withdraw()`
- Withdraw with rules

#### `fundTransfer()`
- Transfer between accounts

---

### 📜 Logging Functions

#### `logTransaction()`
- Writes to `transactions.txt`

#### `viewFullHistory()`
- Shows all transactions

#### `miniStatement()`
- Shows last 5 transactions

---

### 🏧 ATM Function

#### `atmMenu()`
- Simulates ATM behavior

---

### 💼 FD Functions

#### `createFD()`
- Creates fixed deposit

#### `closeFD()`
- Closes FD with penalty logic

---

### 📊 Report Functions

#### `accountSummary()`
- Shows:
- Total accounts
- Total balance
- Breakdown

---

### 💾 Backup Functions

#### `backupData()`
- Copies all files

#### `restoreData()`
- Restores from backup

---
