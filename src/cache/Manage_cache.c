#include "Manage_cache.h"
#include "../HTTP/HttpParser.h"
#include "../HTTP/http_peer/utils/enumToString.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Genera un hash simple y estable para convertir la cache_key en nombre de archivo. */
static unsigned long cache_hash_key(const char *text) {
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char) *text++) != 0) {
        hash = ((hash << 5) + hash) + (unsigned long) c;
    }

    return hash;
}

/* Crea el directorio de cache si no existe y valida que realmente sea un directorio. */
static int ensure_cache_directory(const char *cache_dir) {
    struct stat st;

    if (stat(cache_dir, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }

    if (mkdir(cache_dir, 0777) == 0) {
        return 0;
    }

    return errno == EEXIST ? 0 : -1;
}

/* Crea el archivo indice si todavia no existe. */
static int ensure_index_file(const char *index_path) {
    FILE *index_file = fopen(index_path, "a");

    if (index_file == NULL) {
        return -1;
    }

    fclose(index_file);
    return 0;
}

/* Guarda la respuesta HTTP completa en disco usando modo binario. */
static int write_cache_file(const char *file_path, const char *response_data, size_t response_size) {
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

/* Convierte una linea del indice al struct cache_entry_t. */
static int parse_index_line(const char *line, cache_entry_t *entry) {
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

/* Reescribe el indice usando un archivo temporal para eliminar una clave o entradas expiradas. */
static int rewrite_index(cache_store_t *store, const char *cache_key, int remove_expired_only, int *removed_any) {
    FILE *index_file;
    FILE *temp_file;
    char line[CACHE_KEY_MAX + CACHE_PATH_MAX + 64];
    char temp_path[CACHE_PATH_MAX];
    time_t now = time(NULL);
    int removed = 0;

    snprintf(temp_path, sizeof(temp_path), "%s.tmp", store->index_path);

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

/* Construye una clave unica con metodo + host + URI. */
int http_build_cache_key(const Request *request, char *out, size_t out_size) {
    const char *host;
    const char *method;
    int key_len;

    if (request == NULL || out == NULL || out_size == 0 || request->requestURI == NULL) {
        return -1;
    }

    host = http_request_get_header(request, "Host");
    method = methodToString(request->method);

    if (host == NULL || method == NULL) {
        return -1;
    }

    key_len = snprintf(out, out_size, "%s|%s|%s", method, host, request->requestURI);
    if (key_len < 0 || (size_t) key_len >= out_size) {
        return -1;
    }

    return 0;
}

/* Decide si una peticion es cacheable segun reglas simples de HTTP. */
int http_request_is_cacheable(const Request *request) {
    if (request == NULL || request->headerList == NULL) {
        return IS_NOT_CACHEABLE;
    }

    if (request->method != METHOD_GET && request->method != METHOD_HEAD) {
        return IS_NOT_CACHEABLE;
    }

    for (size_t i = 0; i < request->headerList->count; i++) {
        if (request->headerList->headers[i].type == HEADER_AUTHORIZATION ||
            request->headerList->headers[i].type == HEADER_COOKIE) {
            return IS_NOT_CACHEABLE;
        }

        if (request->headerList->headers[i].type == HEADER_CACHE_CONTROL) {
            if (strstr(request->headerList->headers[i].value, "no-store") != NULL ||
                strstr(request->headerList->headers[i].value, "private") != NULL ||
                strstr(request->headerList->headers[i].value, "max-age=0") != NULL) {
                return IS_NOT_CACHEABLE;
            }
        }
    }

    return IS_CACHEABLE;
}

/* Inicializa la cache persistente sin borrar archivos previos. */
cache_result_t cache_init(cache_store_t *store, const char *cache_dir, int default_ttl) {
    if (store == NULL || cache_dir == NULL || default_ttl <= 0) {
        return CACHE_ERROR;
    }

    memset(store, 0, sizeof(*store));
    snprintf(store->cache_dir, sizeof(store->cache_dir), "%s", cache_dir);
    snprintf(store->index_path, sizeof(store->index_path), "%s/cache_index.txt", cache_dir);
    store->default_ttl = default_ttl;

    if (pthread_mutex_init(&store->mutex, NULL) != 0) {
        return CACHE_ERROR;
    }

    if (ensure_cache_directory(store->cache_dir) != 0) {
        pthread_mutex_destroy(&store->mutex);
        return CACHE_ERROR;
    }

    if (ensure_index_file(store->index_path) != 0) {
        pthread_mutex_destroy(&store->mutex);
        return CACHE_ERROR;
    }

    return CACHE_SUCCESS;
}

/* Busca una clave en el indice y aplica validacion perezosa del TTL. */
cache_result_t cache_lookup(cache_store_t *store, const char *cache_key, char *out_file_path, size_t out_size) {
    FILE *index_file;
    char line[CACHE_KEY_MAX + CACHE_PATH_MAX + 64];
    time_t now = time(NULL);

    if (store == NULL || cache_key == NULL || out_file_path == NULL || out_size == 0) {
        return CACHE_ERROR;
    }

    pthread_mutex_lock(&store->mutex);

    index_file = fopen(store->index_path, "r");
    if (index_file == NULL) {
        pthread_mutex_unlock(&store->mutex);
        return CACHE_ERROR;
    }

    while (fgets(line, sizeof(line), index_file) != NULL) {
        cache_entry_t entry;

        if (parse_index_line(line, &entry) != 0) {
            continue;
        }

        if (strcmp(entry.key, cache_key) != 0) {
            continue;
        }

        fclose(index_file);

        if (entry.expires_at <= now) {
            cache_result_t delete_result;

            pthread_mutex_unlock(&store->mutex);
            delete_result = cache_delete(store, cache_key);
            return delete_result == CACHE_ERROR ? CACHE_ERROR : CACHE_EXPIRED;
        }

        snprintf(out_file_path, out_size, "%s", entry.file_path);
        pthread_mutex_unlock(&store->mutex);
        return CACHE_SUCCESS;
    }

    fclose(index_file);
    pthread_mutex_unlock(&store->mutex);
    return CACHE_NOT_FOUND;
}

/* Guarda la respuesta en un archivo nuevo y registra su vencimiento en el indice. */
cache_result_t cache_save(cache_store_t *store, const char *cache_key, const char *response_data, size_t response_size) {
    FILE *index_file;
    char file_path[CACHE_PATH_MAX];
    time_t expires_at;
    unsigned long hash;

    if (store == NULL || cache_key == NULL || response_data == NULL) {
        return CACHE_ERROR;
    }

    hash = cache_hash_key(cache_key);
    snprintf(file_path, sizeof(file_path), "%s/%lu.cache", store->cache_dir, hash);
    expires_at = time(NULL) + store->default_ttl;

    cache_delete(store, cache_key);

    pthread_mutex_lock(&store->mutex);

    if (write_cache_file(file_path, response_data, response_size) != 0) {
        pthread_mutex_unlock(&store->mutex);
        return CACHE_ERROR;
    }

    index_file = fopen(store->index_path, "a");
    if (index_file == NULL) {
        remove(file_path);
        pthread_mutex_unlock(&store->mutex);
        return CACHE_ERROR;
    }

    fprintf(index_file, "%lld|%s|%s\n", (long long) expires_at, file_path, cache_key);
    fclose(index_file);
    pthread_mutex_unlock(&store->mutex);
    return CACHE_SUCCESS;
}

/* Elimina una entrada del indice y tambien su archivo fisico. */
cache_result_t cache_delete(cache_store_t *store, const char *cache_key) {
    int removed_any = 0;

    if (store == NULL || cache_key == NULL) {
        return CACHE_ERROR;
    }

    pthread_mutex_lock(&store->mutex);

    if (rewrite_index(store, cache_key, 0, &removed_any) != 0) {
        pthread_mutex_unlock(&store->mutex);
        return CACHE_ERROR;
    }

    pthread_mutex_unlock(&store->mutex);
    return removed_any ? CACHE_SUCCESS : CACHE_NOT_FOUND;
}

/* Recorre el indice completo y elimina recursos cuyo TTL ya vencio. */
cache_result_t cache_cleanup_expired(cache_store_t *store) {
    int removed_any = 0;

    if (store == NULL) {
        return CACHE_ERROR;
    }

    pthread_mutex_lock(&store->mutex);

    if (rewrite_index(store, NULL, 1, &removed_any) != 0) {
        pthread_mutex_unlock(&store->mutex);
        return CACHE_ERROR;
    }

    pthread_mutex_unlock(&store->mutex);
    return removed_any ? CACHE_SUCCESS : CACHE_NOT_FOUND;
}

/* Hilo en segundo plano que ejecuta limpieza activa periodicamente. */
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

/* Crea y desacopla el hilo limpiador para que corra en background. */
cache_result_t cache_start_cleaner_thread(cache_store_t *store) {
    pthread_t cleaner_thread;

    if (store == NULL) {
        return CACHE_ERROR;
    }

    if (pthread_create(&cleaner_thread, NULL, cache_cleaner_thread, store) != 0) {
        return CACHE_ERROR;
    }

    pthread_detach(cleaner_thread);
    return CACHE_SUCCESS;
}
