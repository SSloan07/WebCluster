#include "cache_utils.h"
#include "../Manage_cache.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

unsigned long cache_hash_key(const char *text) {
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char) *text++) != 0) {
        hash = ((hash << 5) + hash) + (unsigned long) c;
    }

    return hash;
}

int ensure_cache_directory(const char *cache_dir) {
    struct stat st;

    if (stat(cache_dir, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }

    if (mkdir(cache_dir, 0777) == 0) {
        return 0;
    }

    return errno == EEXIST ? 0 : -1;
}

int ensure_index_file(const char *index_path) {
    FILE *index_file = fopen(index_path, "a");

    if (index_file == NULL) {
        return -1;
    }

    fclose(index_file);
    return 0;
}

int write_cache_file(const char *file_path, const char *response_data, size_t response_size) {
    FILE *cache_file = fopen(file_path, "wb");

    if (cache_file == NULL) {
        return -1;
    }

    if (response_size > 0 && fwrite(response_data, 1, response_size, cache_file) != response_size) {
        fclose(cache_file);
        return -1;
    }

    fclose(cache_file);
    return 0;
}

int parse_index_line(const char *line, cache_entry_t *entry) {
    char buffer[CACHE_KEY_MAX + CACHE_PATH_MAX + 64];
    char *expires_text;
    char *file_path;
    char *cache_key;
    char *newline;

    if (line == NULL || entry == NULL) {
        return -1;
    }

    snprintf(buffer, sizeof(buffer), "%s", line);
    newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }

    expires_text = strtok(buffer, "|");
    file_path = strtok(NULL, "|");
    cache_key = strtok(NULL, "");

    if (expires_text == NULL || file_path == NULL || cache_key == NULL) {
        return -1;
    }

    entry->expires_at = (time_t) atoll(expires_text);
    snprintf(entry->file_path, sizeof(entry->file_path), "%s", file_path);
    snprintf(entry->key, sizeof(entry->key), "%s", cache_key);
    return 0;
}

int rewrite_index(cache_store_t *store, const char *cache_key, int remove_expired_only, int *removed_any) {
    FILE *index_file;
    FILE *temp_file;
    char line[CACHE_KEY_MAX + CACHE_PATH_MAX + 64];
    char temp_path[CACHE_PATH_MAX + 5];
    time_t now = time(NULL);
    int removed = 0;
    int written;

    written = snprintf(temp_path, sizeof(temp_path), "%s.tmp", store->index_path);
    if (written < 0 || written >= (int) sizeof(temp_path)) {
        return -1;
    }

    index_file = fopen(store->index_path, "r");
    if (index_file == NULL) {
        return -1;
    }

    temp_file = fopen(temp_path, "w");
    if (temp_file == NULL) {
        fclose(index_file);
        return -1;
    }

    while (fgets(line, sizeof(line), index_file) != NULL) {
        cache_entry_t entry;
        int should_remove = 0;

        if (parse_index_line(line, &entry) != 0) {
            continue;
        }

        if (remove_expired_only) {
            should_remove = (entry.expires_at <= now);
        } else if (cache_key != NULL && strcmp(entry.key, cache_key) == 0) {
            should_remove = 1;
        }

        if (should_remove) {
            remove(entry.file_path);
            removed = 1;
            continue;
        }

        fputs(line, temp_file);
    }

    fclose(index_file);
    fclose(temp_file);

    if (rename(temp_path, store->index_path) != 0) {
        remove(temp_path);
        return -1;
    }

    if (removed_any != NULL) {
        *removed_any = removed;
    }

    return 0;
}

void *cache_cleaner_thread(void *arg) {
    cache_store_t *store = (cache_store_t *) arg;
    int interval;

    if (store == NULL) {
        return NULL;
    }

    interval = store->default_ttl / 2;
    if (interval < 5) {
        interval = 5;
    }

    while (1) {
        sleep((unsigned int) interval);
        cache_cleanup_expired(store);
    }

    return NULL;
}
