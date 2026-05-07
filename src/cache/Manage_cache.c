#include "Manage_cache.h"
#include "utils/cache_utils.h"
#include "../HTTP/HttpParser.h"
#include "../HTTP/http_peer/utils/enumToString.h"
#include <stdio.h>
#include <string.h>

// Construye una clave unica con metodo + host + URI. 
cache_result_t http_build_cache_key(const Request *request, char *out, size_t out_size) {
    const char *host;
    const char *method;
    int key_len;

    if (request == NULL || out == NULL || out_size == 0 || request->requestURI == NULL) {
        return CACHE_ERROR;
    }

    host = http_request_get_header(request, "Host");
    method = methodToString(request->method);

    if (host == NULL || method == NULL) {
        return CACHE_ERROR;
    }

    key_len = snprintf(out, out_size, "%s|%s|%s", method, host, request->requestURI);
    if (key_len < 0 || (size_t) key_len >= out_size) {
        return CACHE_ERROR;
    }

    return CACHE_SUCCESS;
}

// Decide si una peticion es cacheable segun reglas simples de HTTP. 
cache_result_t http_request_is_cacheable(const Request *request) {
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

// Inicializa la cache persistente sin borrar archivos previos.
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

// Busca una clave en el indice y aplica validacion perezosa del TTL.
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

// Guarda la respuesta en un archivo nuevo y registra su vencimiento en el indice.
cache_result_t cache_save(cache_store_t *store, const char *cache_key, const char *response_data, size_t response_size) {
    FILE *index_file;
    char file_path[CACHE_PATH_MAX];
    time_t expires_at;
    unsigned long hash;
    int written;

    if (store == NULL || cache_key == NULL || response_data == NULL) {
        return CACHE_ERROR;
    }

    hash = cache_hash_key(cache_key);
    written = snprintf(file_path, sizeof(file_path), "%s/%lu.cache", store->cache_dir, hash);
    if (written < 0 || written >= (int) sizeof(file_path)) {
        return CACHE_ERROR;
    }

    expires_at = time(NULL) + store->default_ttl;

    cache_delete(store, cache_key);

    pthread_mutex_lock(&store->mutex);

    if (write_cache_file(file_path, response_data, response_size) != 0) {
        pthread_mutex_unlock(&store->mutex);
        return CACHE_ERROR;
    }
    // índice de la respuesta guardada en disco, con formato: expires_at|file_path|cache_key, esto para poder hacer la busqueda del archivo, eliminar o actualizar mas adelante
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

// Elimina una entrada del indice y tambien su archivo fisico.
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

// Recorre el indice completo y elimina recursos cuyo TTL ya vencio.
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

// Crea y desacopla el hilo limpiador para que corra en background.
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
