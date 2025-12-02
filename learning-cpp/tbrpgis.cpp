#include <iostream>

class Player {
private:
    int health_ = 100;
    std::string name_;

public:
    Player(std::string name) : name_(name) {}
};

class Item {
public:
    virtual void use() = 0;
    virtual std::string getDescription() const = 0;
};

class Potion : Item {
private:
    std::string description_ = "Example potion description";

public:
    void use() override {
        
    }

    std::string getDescription() const override {
        return description_;
    }
};