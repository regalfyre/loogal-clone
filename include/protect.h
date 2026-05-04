#ifndef LOOGAL_PROTECT_H
#define LOOGAL_PROTECT_H

int loogal_path_starts_with(const char *path, const char *prefix);
int loogal_is_protected_path(const char *path, char **protects, int protect_count);

#endif
