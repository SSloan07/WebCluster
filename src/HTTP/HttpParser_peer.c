#include "HttpParser.h"
#include "HttpParser.h"
#include "http_peer/requestParser.h"
#include "http_peer/utils/enumToString.h"
#include "http_peer/structs/Request.h"
#include <string.h>
#include <stdlib.h>
/*
 * Aquí se adapta el parser del compañero a la interfaz del proyecto.
 * Las firmas NO cambian.
 */
// Se realiza la adaptación de la función de mi compañero con el adaptador de manage_client.c
http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    http_request_t *request

) 
{
    // Utilzo el parser de mi compañero para llenar la estructura Request, luego adapto esa estructura a http_request_t para que el resto del proyecto pueda usarla sin problemas.

    Request *req = createRequest(); // Defino la estructura utilizada por el parser de mi compañero
    if (req == NULL) {
        printf("Ni la  estructura  Request se creó bien\n");
        return HTTP_PARSE_ERROR;
    }
    
    size_t position = 0;

    int parseReqLine = parseRequestLine(buffer, len, req , &position);

    if(parseReqLine == -1){
        printf("Mi rey, faltaron argumentos en el request line\n");
        freeRequest(req);
        return HTTP_PARSE_ERROR;
    }
    // Se utiliza el parseo de los headers
    int parseHead = parseHeaders(buffer, len, req , &position);
    
    if(parseHead == -1){
        printf("Mi rey, faltaron argumentos en los headers\n");
        freeRequest(req);
        return HTTP_PARSE_ERROR;
    }

    if(req->method == METHOD_POST) {
        int parseBod = parseBody(buffer , len , req , &position);

        if(parseBod == -1){
            printf("Mi rey, faltaron argumentos en el body\n");
            freeRequest(req);
            return HTTP_PARSE_ERROR;
        }
    }
    // Convertir la estructura de request a http_request_t
    
    // Convertir método: enum → string
    const char *method_str = methodToString(req->method);
    strncpy(request->method, method_str, HTTP_METHOD_MAX - 1);
    request->method[HTTP_METHOD_MAX - 1] = '\0';
    
    // Convertir URI: copy directo (ya es string)
    strncpy(request->request_uri, req->requestURI, HTTP_URI_MAX - 1);
    request->request_uri[HTTP_URI_MAX - 1] = '\0';
    
    // Convertir versión: enum → string
    const char *version_str = versionToString(req->httpVersion);
    strncpy(request->http_version, version_str, HTTP_VERSION_MAX - 1);
    request->http_version[HTTP_VERSION_MAX - 1] = '\0';
    
   
    //convierto Request_HeaderList → http_header_t[]
    
    request->header_count = 0;
    
    for (size_t i = 0; i < req->headerList->count && i < HTTP_MAX_HEADERS; i++) {
        // Obtener nombre del header desde el enum
        const char *header_name = headerToString(req->headerList->headers[i].name);
        
        // Copiar nombre
        strncpy(request->headers[i].key, header_name, HTTP_HEADER_KEY_MAX - 1);
        request->headers[i].key[HTTP_HEADER_KEY_MAX - 1] = '\0';
        
        // Copiar valor
        strncpy(request->headers[i].value, req->headerList->headers[i].value, HTTP_HEADER_VALUE_MAX - 1);
        request->headers[i].value[HTTP_HEADER_VALUE_MAX - 1] = '\0';
        
        request->header_count++;
    }
    
   
    // Lo mismo con body 
  
    if(req->body != NULL && req->bodyLength > 0) {
        request->body = malloc(req->bodyLength);
        if(request->body == NULL) {
            printf("Error: malloc para body falló\n");
            freeRequest(req);
            return HTTP_PARSE_ERROR;
        }
        memcpy(request->body, req->body, req->bodyLength);
        request->body_length = req->bodyLength;
    } else {
        request->body = NULL;
        request->body_length = 0;
    }
    
    // Marcar como válido
    request->is_valid = 1;
    
    // Liberar la estructura temporal de mi compañero
    freeRequest(req);
    
    return HTTP_PARSE_OK;
}


http_parse_result_t http_parse_response(
    const char *buffer,
    size_t len,
    http_response_t *response
) {



    return HTTP_PARSE_ERROR;
}

const char *http_request_get_header(const http_request_t *request, const char *key) {

    if (!request || !key) {
        return NULL;
    }
    
    // Recorrer todos los headers parseados
    for (int i = 0; i < request->header_count; i++) {
        // strcasecmp compara ignorando mayúsculas/minúsculas
        if (strcasecmp(request->headers[i].key, key) == 0) {
            return request->headers[i].value;
        }
    }
    
    return NULL;  // Header no encontrado
}

const char *http_response_get_header(const http_response_t *response, const char *key) {
    return NULL;
}

int http_request_is_method_supported(const http_request_t *request) {
    if (!request || !request->is_valid) return 0;

    return strcmp(request->method, "GET") == 0 ||
           strcmp(request->method, "HEAD") == 0 ||
           strcmp(request->method, "POST") == 0;
    return 0;
}

int http_request_is_cacheable(const http_request_t *request) {
    return 0;
}

int http_build_cache_key(const http_request_t *request, char *out, size_t out_size) {
    return -1;
}

void http_request_free(http_request_t *request) {
    if (!request) return;
    free(request->body);
    request->body = NULL;
    request->body_length = 0;
    request->is_valid = 0;
    (void)request;
}

void http_response_free(http_response_t *response) {
    (void)response;
}