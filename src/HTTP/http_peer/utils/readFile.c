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

int buildDocumentPath(const char *filePath, char *outPath, size_t outSize) {
    int written;

    if (filePath == NULL || outPath == NULL || outSize == 0) {
        return -1;
    }

    written = snprintf(outPath, outSize, "%s%s", basePath, filePath);
    if (written < 0 || written >= (int) outSize) {
        return -1;
    }

    return 0;
}

char *readFile(const char *filePath , size_t *outSize){

    char fullpath[512];
    if (buildDocumentPath(filePath, fullpath, sizeof(fullpath)) != 0) {
        *outSize = 0;
        return NULL;
    }

    FILE *file = fopen(fullpath , "rb");

    if(file == NULL){
        printf("No se pudo abrir %s\n" , fullpath);
        *outSize = 0;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file , 0 , SEEK_SET);

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
    const char *ext = strrchr(filePath, '.');
    if (ext == NULL) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".txt") == 0) return "text/plain";

    return "application/octet-stream";
}
