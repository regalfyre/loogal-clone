#ifndef LOOGAL_MANIFEST_H
#define LOOGAL_MANIFEST_H

#include "loogal.h"
#include <stdio.h>

int loogal_manifest_write_move(
    FILE *manifest,
    const LoogalRecord *move,
    const LoogalRecord *keep,
    const char *new_path,
    const char *filename,
    int similarity_percent
);

#endif
