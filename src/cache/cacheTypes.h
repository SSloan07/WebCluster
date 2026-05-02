#ifndef CACHE_TYPES_H
#define CACHE_TYPES_H

#include <pthread.h>
#include <stddef.h>
#include <time.h>

#define CACHE_KEY_MAX 2048
#define CACHE_PATH_MAX 1024
#define CACHE_MAX_ENTRIES 100

typedef enum {
    CACHE_SUCCESS,
    CACHE_NOT_FOUND,
    CACHE_EXPIRED,
    CACHE_ERROR,
    IS_NOT_CACHEABLE,
    IS_CACHEABLE
} cache_result_t;

typedef struct {
    char key[CACHE_KEY_MAX];
    char file_path[CACHE_PATH_MAX];
    time_t expires_at;
} cache_entry_t;

typedef struct {
    char cache_dir[CACHE_PATH_MAX];
    char index_path[CACHE_PATH_MAX];
    int default_ttl;
    pthread_mutex_t mutex;
} cache_store_t;

#endif
