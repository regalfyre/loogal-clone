#ifndef LOOGAL_THUMBNAIL_H
#define LOOGAL_THUMBNAIL_H

#include <stddef.h>

int cmd_thumbnail(int argc, char **argv);
int loogal_thumbnail_cache_path(const char *image_path, int size, char *out, size_t out_sz);

#endif
