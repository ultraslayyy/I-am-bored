#include <string.h>
#include "sa/string_view.h"

int sv_equals(StringView a, StringView b) {
    if (a.length != b.length) return 0;
    return memcmp(a.data, b.data, a.length) == 0;
}