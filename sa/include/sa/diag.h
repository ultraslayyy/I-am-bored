#pragma once
#include <stddef.h>
#include "span.h"

typedef enum {
    DIAG_NOTE,
    DIAG_WARNING,
    DIAG_ERROR
} DiagnosticKind;

typedef struct {
    DiagnosticKind kind;
    Span span;
    const char *message;
} Diagnostic;