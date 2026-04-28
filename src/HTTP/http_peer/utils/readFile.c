#include "readFile.h"
#include <stdio.h>
#include <string.h>

char *BASE_PATH = "/home/alejo/Universidad/ProyectoTelematica1";

char *readFile(const char *filePath , size_t *outSize){

    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", BASE_PATH, filePath);

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

    char *content = malloc(size);
    size_t bytesRead = fread(content, 1, size, file);
    fclose(file);

    *outSize = bytesRead;
    return content;
}

int fileExist(const char *filePath){

    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", BASE_PATH, filePath);
    FILE *file = fopen(fullpath , "rb");

    if(file == NULL){
        return -1;
    }

    return 0;
}

const char *getContentType(const char *filePath) {
    const char *ext = strrchr(filePath, '.');  // Ultima aparición de '.' y se trae el resto
    if (ext == NULL) return "application/octet-stream";  // Generico
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    
    return "application/octet-stream";  // binario genérico
}