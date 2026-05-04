#ifndef LOOGAL_WINDOW_H
#define LOOGAL_WINDOW_H

#define LOOGAL_WINDOW_PAGE_LIMIT 10
#define LOOGAL_WINDOW_MAX_RESULTS 64
#define LOOGAL_WINDOW_PATH_MAX 2048

typedef struct {
    int rank;
    char path[LOOGAL_WINDOW_PATH_MAX];
    char label[32];
    double similarity_percent;
    int hash_distance;
    int width;
    int height;
    long long file_size_bytes;
} LoogalWindowResult;

typedef struct {
    char session_id[256];
    int offset;
    int limit;
    int returned;
    int total_hits;
    LoogalWindowResult results[LOOGAL_WINDOW_MAX_RESULTS];
} LoogalWindowPage;

#endif
