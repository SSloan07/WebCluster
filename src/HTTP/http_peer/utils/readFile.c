#include "readFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *basePath = ".";

void setDocumentRoot(const char *rootPath) {
    if (rootPath != NULL && rootPath[0] != '\0') {
        basePath = rootPath;
    }
}

char *readFile(const char *filePath , size_t *outSize){

    char fullpath[512];
    int written = snprintf(fullpath, sizeof(fullpath), "%s%s", basePath, filePath);
    if (written < 0 || written >= (int)sizeof(fullpath)) {
        *outSize = 0;
        return NULL;
    }

    FILE *file = fopen(fullpath , "rb"); // Leemos un binario

    if(file == NULL){
        printf("No se pudo abrir %s\n" , fullpath);
        *outSize = 0;
        return NULL;
    }

    fseek(file, 0, SEEK_END); // Ir al final para saber el tamano del archivo
    long size = ftell(file); // Retorna la posicion en la que estamos parados
    fseek(file , 0 , SEEK_SET); // Volvemos al incio

    if (size < 0){
        fclose(file);
        *outSize = 0;
        return NULL;
    }

    char *content = malloc((size_t)size);
    if (content == NULL) {
        fclose(file);
        *outSize = 0;
        return NULL;
    }

    size_t bytesRead = fread(content, 1, size, file);
    fclose(file);

    *outSize = bytesRead;
    return content;
}

const char *getContentType(const char *filePath) {
    const char *ext = strrchr(filePath, '.');  // Ultima aparición de '.' y se trae el resto
    if (ext == NULL) return "application/octet-stream";  // Generico
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    
    return "application/octet-stream";  // binario genérico
}
