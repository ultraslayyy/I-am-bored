#include <utility>
#include <vector>

struct Point {
    int x;
    int y;
};

class MyClass {
public:
    MyClass(int x, int y) {}
};

int x;            // Unitialised
static int x;     // Zero-initialised
int x{};          // Value-initialised: 0
int x = {};       // Value-initialised: 0
int x = int();    // Value-initialised: 0 (temporary then copy)
int x = 10;       // Copy initialisation
int x = int(10);  // Copy initialisation from temporary
int x = (1, 0);   // Comma operator (evaluates to rightmost), x = 0
int x();          // Declares function
int x(10);        // Direct initialisation
int x{10};        // Direct list initialisation (preferred over above)
int x = {10};     // Copy list initialisation
int x = 3.14;     // 3
int x{3.14};      // You get error
auto x = 10;      // int
auto x{10};       // int (good since C++17)
auto x = {10};    // std::initializer_list<int>
auto x = int{10}; // int
auto x = (1, 0);  // int, value = 0 (comma operator)

Point p{1, 2};                     // Aggregate initialisation
Point p = {1, 2};                  // Aggregate initialisation (better)
Point p = {.x = 1, .y = 2};        // Designated initialisers (C++20)
auto [a, b] = std::pair{1, 2};     // Structured binding (C++17)
std::vector<int> v = {1, 2, 3};    // Initialiser list constructor
std::vector<int> v(3, 10);         // 3 elements, all 10
std::vector<int> v{3, 10};         // Elements: 3, 10
auto* p = new MyClass{1, 2};       // Direct list initialisation on heap
MyClass a(1, 2);                   // Direct initialisation
MyClass a{1, 2};                   // List initialisation
MyClass a = {1, 2};                // Copy list initialisation
MyClass a = MyClass(1, 2);         // Copy initialisation
constexpr int x = 42;              // Compile-time constant
auto f = [x = 42]() { return x; }; // Init capture (C++14)

int x() {}