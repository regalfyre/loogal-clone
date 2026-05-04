#include "protect.h"
#include "loogal.h"
#include <string.h>

int loogal_path_starts_with(const char *path, const char *prefix) {
    if (!path || !prefix) return 0;

    size_t n = strlen(prefix);
    if (n == 0) return 0;

    if (strncmp(path, prefix, n) != 0) return 0;

    /*
      Avoid false positives:
      /home/me/Pic should not protect /home/me/Pictures
      Exact match is OK.
      Prefix followed by '/' is OK.
    */
    return path[n] == '\0' || path[n] == '/';
}

int loogal_is_protected_path(const char *path, char **protects, int protect_count) {
    if (!path || !protects || protect_count <= 0) return 0;

    for (int i = 0; i < protect_count; i++) {
        if (loogal_path_starts_with(path, protects[i])) {
            loogal_log("protect", "hit", path);
            return 1;
        }
    }

    return 0;
}
