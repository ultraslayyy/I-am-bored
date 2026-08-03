#include <math.h>
#include <string.h>
#include "funcs/roots.h"
#include "functions.h"

static double fn_sqrt(double *args, size_t count) {
    return m_sqrt(args[0]);
}

static double fn_pow(double *args, size_t count) {
    return m_pow(args[0], args[1]);
}

static double fn_sin(double *args, size_t count) {
    return sin(args[0]);
}

typedef struct {
    const char *name;
    math_function_t function;
} FunctionEntry;

static FunctionEntry functions[] = {
    {"sqrt", fn_sqrt},
    {"pow",  fn_pow},
    {"sin",  fn_sin}
};

double call_function(const char *name, double *args, size_t count, int *success) {
    size_t total = sizeof(functions) / sizeof(functions[0]);

    for (size_t i = 0; i < total; ++i) {
        if (strcmp(name, functions[i].name) == 0) {
            *success = 1;

            return functions[i].function(args, count);
        }
    }

    *success = 0;
    return 0;
}