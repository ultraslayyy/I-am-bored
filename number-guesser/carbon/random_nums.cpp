#include <random>
#include "random_nums.h"

static std::random_device rd;
static std::mt19937 gen(rd());

extern "C" {

int32_t random_number(int32_t min, int32_t max) {
    if (min > max) {
        std::swap(min, max);
    }
    std::uniform_int_distribution<int32_t> dist(min, max);
    return dist(gen);
}

}