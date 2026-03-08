#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;


// BIT FLAGS - Permission System
const unsigned int CAN_WITHDRAW = 1;   
const unsigned int CAN_DEPOSIT = 2;     
const unsigned int CAN_TRANSFER = 4;  
const unsigned int VIP_ACCOUNT = 8;  

class Account {
protected:
    int accountId;
    string name;
    double balance;
    unsigned int permissions;
    vector<double> transactions;  // + for deposit, - for withdrawal

public:

    Account(int id, string n, double bal, unsigned int perm) {
        accountId = id;
        name = n;
        balance = bal;
        permissions = perm;
    }

    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;
    virtual void saveToFile(ofstream& file) = 0;
    virtual void loadFromFile(ifstream& file) = 0;
    virtual string getType() = 0;

    // Virtual destructor 
    virtual ~Account() {
        cout << "Account " << accountId << " destroyed.\n";
    }

   
    int getId() { return accountId; }
    string getName() { return name; }
    double getBalance() { return balance; }
    unsigned int getPermissions() { return permissions; }
    vector<double> getTransactions() { return transactions; }

    // Check permission using bitwise AND
    bool hasPermission(unsigned int perm) {
        return (permissions & perm) != 0;
    }

    // Show account details
    void showAccount() {
        cout << "\n========== ACCOUNT DETAILS ==========\n";
        cout << "Type: " << getType() << endl;
        cout << "ID: " << accountId << endl;
        cout << "Name: " << name << endl;
        cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
        
        cout << "Permissions: ";
        if (hasPermission(CAN_WITHDRAW)) cout << "Withdraw ";
        if (hasPermission(CAN_DEPOSIT)) cout << "Deposit ";
        if (hasPermission(CAN_TRANSFER)) cout << "Transfer ";
        if (hasPermission(VIP_ACCOUNT)) cout << "VIP";
        cout << endl;

        cout << "Transaction History:\n";
        if (transactions.empty()) {
            cout << "  No transactions yet.\n";
        } else {
            for (int i = 0; i < transactions.size(); i++) {
                if (transactions[i] > 0) {
                    cout << "  +" << transactions[i] << " (Deposit)\n";
                } else {
                    cout << "  " << transactions[i] << " (Withdrawal)\n";
                }
            }
        }
        cout << "=====================================\n";
    }

    // Add transaction to history
    void addTransaction(double amount) {
        transactions.push_back(amount);
    }
};

//Derived classes
class SavingsAccount : public Account {
private:
    double interestRate;  

public:
    SavingsAccount(int id, string n, double bal, unsigned int perm) 
        : Account(id, n, bal, perm) {
        interestRate = 0.05;  
    }

    string getType() override {
        return "Savings";
    }

    void deposit(double amount) override {
        
        if (!(permissions & CAN_DEPOSIT)) {
            cout << "ERROR: Deposit permission denied!\n";
            return;
        }
        if (amount <= 0) {
            cout << "ERROR: Invalid amount!\n";
            return;
        }
        balance = balance + amount;
        addTransaction(amount);  // Positive for deposit
        cout << "Deposited $" << amount << " to Savings Account.\n";
    }

    void withdraw(double amount) override {
        
        if (!(permissions & CAN_WITHDRAW)) {
            cout << "ERROR: Withdraw permission denied!\n";
            return;
        }
        if (amount <= 0) {
            cout << "ERROR: Invalid amount!\n";
            return;
        }
        if (amount > balance) {
            cout << "ERROR: Insufficient funds!\n";
            return;
        }
        balance = balance - amount;
        addTransaction(-amount);  // Negative for withdrawal
        cout << "Withdrawn $" << amount << " from Savings Account.\n";
    }

    void saveToFile(ofstream& file) override {
        file << "ACCOUNT Savings\n";
        file << accountId << " " << name << " " << balance << " " << permissions << "\n";
        file << "TRANSACTIONS\n";
        for (int i = 0; i < transactions.size(); i++) {
            file << transactions[i] << "\n";
        }
    }

    void loadFromFile(ifstream& file) override {
   
        string line;
        file >> line;  
        
        double trans;
        while (file >> trans) {
            // Check if next line is "ACCOUNT" (new account)
            if (file.peek() == 'A') {
                break;
            }
            transactions.push_back(trans);
        }
    }

  
    void calculateInterest() {
        double interest = balance * interestRate;
        cout << "Interest earned: $" << interest << endl;
    }
};

class CurrentAccount : public Account {
private:
    double overdraftLimit;  

public:
    CurrentAccount(int id, string n, double bal, unsigned int perm) 
        : Account(id, n, bal, perm) {
        overdraftLimit = 1000.0;  // $1000 overdraft allowed
    }

    string getType() override {
        return "Current";
    }

    void deposit(double amount) override {
        // Check permission using bitwise AND
        if (!(permissions & CAN_DEPOSIT)) {
            cout << "ERROR: Deposit permission denied!\n";
            return;
        }
        if (amount <= 0) {
            cout << "ERROR: Invalid amount!\n";
            return;
        }
        balance = balance + amount;
        addTransaction(amount);
        cout << "Deposited $" << amount << " to Current Account.\n";
    }

    void withdraw(double amount) override {
        if (!(permissions & CAN_WITHDRAW)) {
            cout << "ERROR: Withdraw permission denied!\n";
            return;
        }
        if (amount <= 0) {
            cout << "ERROR: Invalid amount!\n";
            return;
        }
        // Check balance + overdraft
        if (amount > balance + overdraftLimit) {
            cout << "ERROR: Exceeds overdraft limit!\n";
            return;
        }
        balance = balance - amount;
        addTransaction(-amount);
        cout << "Withdrawn $" << amount << " from Current Account.\n";
    }

    void saveToFile(ofstream& file) override {
        file << "ACCOUNT Current\n";
        file << accountId << " " << name << " " << balance << " " << permissions << "\n";
        file << "TRANSACTIONS\n";
        for (int i = 0; i < transactions.size(); i++) {
            file << transactions[i] << "\n";
        }
    }

    void loadFromFile(ifstream& file) override {
        string line;
        file >> line;  
        
        double trans;
        while (file >> trans) {
            if (file.peek() == 'A') {
                break;
            }
            transactions.push_back(trans);
        }
    }
};


class BankSystem {
private:
    vector<Account*> accounts;  // Store pointers to accounts
    double monthlyTotals[12];   // Array for monthly summary

public:
    BankSystem() {
        // Initialize array to 0
        for (int i = 0; i < 12; i++) {
            monthlyTotals[i] = 0;
        }
    }

    ~BankSystem() {
        // Clean up all dynamic memory
        cout << "\nCleaning up memory...\n";
        for (int i = 0; i < accounts.size(); i++) {
            delete accounts[i];  
        }
        accounts.clear();
    }

   
    void createAccount() {
        int choice, id;
        string name;
        double balance;
        unsigned int perm = 0;

        cout << "\n--- Create Account ---\n";
        cout << "1. Savings Account\n";
        cout << "2. Current Account\n";
        cout << "Enter choice: ";
        cin >> choice;

        cout << "Enter Account ID: ";
        cin >> id;

        // Check if ID already exists
        for (int i = 0; i < accounts.size(); i++) {
            if (accounts[i]->getId() == id) {
                cout << "ERROR: Account ID already exists!\n";
                return;
            }
        }

        cout << "Enter Name: ";
        cin.ignore();
        getline(cin, name);

        cout << "Enter Initial Balance: ";
        cin >> balance;

        // Set permissions using bitwise OR
        cout << "\nSet Permissions:\n";
        char ans;

        cout << "Allow Withdraw? (y/n): ";
        cin >> ans;
        if (ans == 'y') perm = perm | CAN_WITHDRAW;  // Bitwise OR

        cout << "Allow Deposit? (y/n): ";
        cin >> ans;
        if (ans == 'y') perm = perm | CAN_DEPOSIT;   // Bitwise OR

        cout << "Allow Transfer? (y/n): ";
        cin >> ans;
        if (ans == 'y') perm = perm | CAN_TRANSFER;  // Bitwise OR

        cout << "VIP Account? (y/n): ";
        cin >> ans;
        if (ans == 'y') perm = perm | VIP_ACCOUNT;   // Bitwise OR

        // Create account dynamically based on choice
        Account* newAcc = nullptr;
        if (choice == 1) {
            newAcc = new SavingsAccount(id, name, balance, perm);
        } else if (choice == 2) {
            newAcc = new CurrentAccount(id, name, balance, perm);
        } else {
            cout << "Invalid choice!\n";
            return;
        }

        accounts.push_back(newAcc);
        cout << "Account created successfully!\n";
    }

    // Find account by ID
    Account* findAccount(int id) {
        for (int i = 0; i < accounts.size(); i++) {
            if (accounts[i]->getId() == id) {
                return accounts[i];
            }
        }
        return nullptr;
    }

   
    void deposit() {
        int id;
        double amount;
        
        cout << "\nEnter Account ID: ";
        cin >> id;

        Account* acc = findAccount(id);
        if (acc == nullptr) {
            cout << "Account not found!\n";
            return;
        }

        cout << "Enter amount to deposit: ";
        cin >> amount;

        acc->deposit(amount);  
    }

  
    void withdraw() {
        int id;
        double amount;
        
        cout << "\nEnter Account ID: ";
        cin >> id;

        Account* acc = findAccount(id);
        if (acc == nullptr) {
            cout << "Account not found!\n";
            return;
        }

        cout << "Enter amount to withdraw: ";
        cin >> amount;

        acc->withdraw(amount);  
    }

   
    void showAccount() {
        int id;
        cout << "\nEnter Account ID: ";
        cin >> id;

        Account* acc = findAccount(id);
        if (acc == nullptr) {
            cout << "Account not found!\n";
            return;
        }

        acc->showAccount();
    }

    // Transfer between accounts (bonus feature)
    void transfer() {
        int fromId, toId;
        double amount;

        cout << "\n--- Transfer Money ---\n";
        cout << "From Account ID: ";
        cin >> fromId;
        cout << "To Account ID: ";
        cin >> toId;

        Account* fromAcc = findAccount(fromId);
        Account* toAcc = findAccount(toId);

        if (fromAcc == nullptr || toAcc == nullptr) {
            cout << "One or both accounts not found!\n";
            return;
        }

        // Check transfer permission using bitwise AND
        if (!(fromAcc->getPermissions() & CAN_TRANSFER)) {
            cout << "ERROR: Transfer permission denied for source account!\n";
            return;
        }

        cout << "Enter amount to transfer: ";
        cin >> amount;

        if (amount <= 0) {
            cout << "ERROR: Invalid amount!\n";
            return;
        }

        // Check if from account has enough balance
        if (amount > fromAcc->getBalance()) {
            cout << "ERROR: Insufficient funds!\n";
            return;
        }

        
        fromAcc->withdraw(amount);
        toAcc->deposit(amount);
        cout << "Transfer successful!\n";
    }

  
    void saveToFile() {
        ofstream file("bank_data.txt");
        if (!file) {
            cout << "ERROR: Cannot open file!\n";
            return;
        }

        for (int i = 0; i < accounts.size(); i++) {
            accounts[i]->saveToFile(file);
            file << "\n";  
        }

        file.close();
        cout << "Data saved to bank_data.txt\n";
    }

    
    void loadFromFile() {
       
        for (int i = 0; i < accounts.size(); i++) {
            delete accounts[i];
        }
        accounts.clear();

        ifstream file("bank_data.txt");
        if (!file) {
            cout << "ERROR: Cannot open file!\n";
            return;
        }

        string line;
        while (file >> line) { 
            if (line != "ACCOUNT") break;

            string type;
            file >> type;  

            int id;
            string name;
            double balance;
            unsigned int perm;

            file >> id >> name >> balance >> perm;

            Account* acc = nullptr;
            if (type == "Savings") {
                acc = new SavingsAccount(id, name, balance, perm);
            } else if (type == "Current") {
                acc = new CurrentAccount(id, name, balance, perm);
            }

            if (acc != nullptr) {
                acc->loadFromFile(file);
                accounts.push_back(acc);
            }
        }

        file.close();
        cout << "Data loaded from bank_data.txt\n";
    }

   
    void showMonthlySummary() {
        cout << "\n--- Monthly Summary ---\n";
        cout << "Note: This is a demo using sample data.\n";
        
       
        monthlyTotals[0] = 5000;   
        monthlyTotals[1] = -2000;  
        monthlyTotals[2] = 3000;  

        string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        for (int i = 0; i < 12; i++) {
            if (monthlyTotals[i] != 0) {
                cout << months[i] << ": $";
                if (monthlyTotals[i] > 0) {
                    cout << "+" << monthlyTotals[i] << " (Net Deposit)\n";
                } else {
                    cout << monthlyTotals[i] << " (Net Withdrawal)\n";
                }
            }
        }
    }
    
    void compressTransaction() {
        cout << "\n--- Transaction Compression Demo ---\n";
        
        unsigned int type;
        double amount;
        
        cout << "Transaction Types:\n";
        cout << "1 = Deposit\n";
        cout << "2 = Withdrawal\n";
        cout << "3 = Transfer\n";
        cout << "Enter type (1-3): ";
        cin >> type;
        
        cout << "Enter amount: ";
        cin >> amount;

        // ENCODE: Pack type (4 bits) and amount (28 bits) into one unsigned int
        // Shift type left by 28 bits to make room for amount
        unsigned int packed = (type << 28) | (unsigned int)amount;
        
        cout << "Original: Type=" << type << ", Amount=" << amount << endl;
        cout << "Packed into: " << packed << " (binary)\n";

        // DECODE: Extract type and amount
        unsigned int extractedType = packed >> 28;        // Shift right to get type
        unsigned int extractedAmount = packed & 0x0FFFFFFF; // Mask to get amount (28 bits of 1s)
        
        cout << "Unpacked: Type=" << extractedType << ", Amount=" << extractedAmount << endl;
    }

    // List all accounts
    void listAllAccounts() {
        cout << "\n--- All Accounts ---\n";
        if (accounts.empty()) {
            cout << "No accounts found.\n";
            return;
        }
        for (int i = 0; i < accounts.size(); i++) {
            cout << accounts[i]->getId() << " - " 
                 << accounts[i]->getName() << " - "
                 << accounts[i]->getType() << " - $"
                 << accounts[i]->getBalance() << endl;
        }
    }
};


int main() {
    BankSystem bank;
    int choice;

    cout << "========================================\n";
    cout << "   SECURE FILE-BASED BANKING SYSTEM\n";
    cout << "========================================\n";

    do {
        cout << "\n--- MAIN MENU ---\n";
        cout << "1. Create Account\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Show Account\n";
        cout << "5. Transfer Money\n";
        cout << "6. List All Accounts\n";
        cout << "7. Save to File\n";
        cout << "8. Load from File\n";
        cout << "9. Monthly Summary\n";
        cout << "10. Compress Transaction (Demo)\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: bank.createAccount(); break;
            case 2: bank.deposit(); break;
            case 3: bank.withdraw(); break;
            case 4: bank.showAccount(); break;
            case 5: bank.transfer(); break;
            case 6: bank.listAllAccounts(); break;
            case 7: bank.saveToFile(); break;
            case 8: bank.loadFromFile(); break;
            case 9: bank.showMonthlySummary(); break;
            case 10: bank.compressTransaction(); break;
            case 0: cout << "Goodbye!\n"; break;
            default: cout << "Invalid choice!\n";
        }
    } while (choice != 0);

    return 0;
}