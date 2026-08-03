#pragma once

#include <stddef.h>

typedef double (*math_function_t)(double *args, size_t count);

double call_function(const char *name, double *args, size_t count, int *success);