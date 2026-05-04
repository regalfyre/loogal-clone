#ifndef LOOGAL_RECEIPT_H
#define LOOGAL_RECEIPT_H

#include "loogal.h"

int loogal_write_move_receipt(
    const LoogalRecord *move,
    const LoogalRecord *keep,
    const char *new_path,
    int similarity_percent
);

#endif
