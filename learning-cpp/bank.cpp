#include <iostream>

class BankAccount {
private:
    double balance_;
    int accountId_;
    static int counter_;

public:
    BankAccount() : balance_(0.0), accountId_(0) {}

    BankAccount(double initialBalance) : accountId_(++counter_) {
        balance_ = initialBalance;
    }

    double getBalance() const {
        return balance_;
    }

    bool withdraw(double amount) {
        if (balance_ >= amount) {
            balance_ -= amount;
            return true;
        } else {
            std::cerr << "Insufficient funds" << std::endl;
            return false;
        }
    }

    void deposit(double amount) {
        balance_ += amount;
    }
};