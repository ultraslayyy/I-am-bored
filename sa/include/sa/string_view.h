#pragma oncce
#include <stddef.h>

typedef struct {
    const char *data;
    size_t length;
} StringView;

int sv_equals(StringView a, StringView b);
int sv_equals_str(StringView a, const char *b);