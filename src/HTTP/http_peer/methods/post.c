#include "post.h"
#include "../utils/readFile.h"

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

# define UPLOAD_DIR "./files"
HTTP_Status HTTPPost(Request *req , HTTP_Response *res){
    
    if(strncmp(req->requestURI , "/files/" , 7) != 0){
        return STATUS_405;
    }

    const char* filename = req->requestURI + 7;

    if(strlen(filename) == 0) return STATUS_400;
    if(strstr(filename , "..") != NULL || strchr(filename , '/')) return STATUS_400; // Impedir que se forme una URI en otros directorios de abajo

    // Contruir el path verdadero con el ../../files/filename
    char fullpath[512];
    int bytesWritten = snprintf(fullpath , sizeof(fullpath) , "%s/%s" , UPLOAD_DIR , filename);
    if(bytesWritten < 0 || bytesWritten >= (int)(sizeof(fullpath))) return STATUS_400; // No logramos copiar nada o se paso del tamaño del path

    mkdir(UPLOAD_DIR, 0755); // Si ya esta creado no pasa nada

    // Crear el archivo
    FILE *f = fopen(fullpath , "wb");
    if(f == NULL) return STATUS_500;
    
    fwrite(req->body , 1 , req->bodyLength, f);
    fclose(f);

    res->content = strdup("File created successfully\r\n");
    res->contentLength = strlen(res->content);

    char lenContent[32];
    snprintf(lenContent, sizeof(lenContent), "%zu", res->contentLength);

    addResponseHeader(res->headerList, "Content-Type", "text/plain");
    addResponseHeader(res->headerList, "Content-Length", lenContent);
    addResponseHeader(res->headerList, "Location", req->requestURI);

    return STATUS_201;
}
