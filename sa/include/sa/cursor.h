#pragma once
#include <source.h>

typedef struct {
    SourceFile *source;
    size_t      position;
} Cursor;

void cursor_init     (      Cursor *cursor, SourceFile *source);
int  cursor_eof      (const Cursor *cursor);
char cursor_peek     (const Cursor *cursor);
char cursor_peek_next(const Cursor *cursor);
char cursor_advance  (      Cursor *cursor);
int  cursor_match    (      Cursor *cursor, char expected);