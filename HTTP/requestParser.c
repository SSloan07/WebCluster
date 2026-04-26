#include "requestParser.h"
#include "utils/stringToEnum.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_URI_LENGTH 2048

int parseRequestLine(const char *rawRequestLine , size_t rawLength, RequestLine *req , size_t *position){

    int count = 0;
    size_t lastLetter = 0;

    // Parsear todos los espacios en blanco y mirar que espacio corresponde a que parte del request
    for(size_t i = 0 ; i < rawLength ; i++){
        
        // Movemos el puntero i+1 veces (Ya que i es el espacio que separa entre expresiones)
        // Al hacer rawRequestLine + lastLetter estamos moviendo lastLetter veces el puntero para que apunte a la ultima letra encontrada
        if(rawRequestLine[i] == ' ' && count == 0){

            size_t length = i - lastLetter;
            char tempMethod[16];

            if(length >= sizeof(tempMethod) || length == 0) return -1;

            memcpy(tempMethod, rawRequestLine + lastLetter , length);
            tempMethod[length] = '\0';
            req->method = getHTTPMethod(tempMethod);
            
            lastLetter = i+1;
            count++;
        }
        
        else if(rawRequestLine[i] == ' ' && count == 1){

            size_t length = i - lastLetter;

            if (length == 0 || length > MAX_URI_LENGTH) return -1;

            req->requestURI = malloc(length + 1);
            memcpy(req->requestURI , rawRequestLine + lastLetter , length);
            req->requestURI[length] = '\0';

            lastLetter = i+1;
            count++;
        }

        else if(rawRequestLine[i] == '\r' && count == 2){

            if(i + 1 >= rawLength || rawRequestLine[i+1] != '\n' ) return -1;

            size_t length = i - lastLetter;
            char tempVersion[32];
            
            if(length >= sizeof(tempVersion) || length == 0 ) return -1;

            memcpy(tempVersion , rawRequestLine + lastLetter , length);
            tempVersion[length] = '\0';
            req->httpVersion = getHTTPVersion(tempVersion);
            
            lastLetter = i+2;
            *position = lastLetter;
            count++;
        }
    }

    if(count != 3){
        printf("No se encontraron los 3 argumentos necesarios del request line\n");
        return -1;
    }

    return 0;
}

int parseHeaders(const char *rawRequestLine , size_t rawLength, RequestLine *req , size_t *position){

    size_t lastLetter = *position;
    int value = 0; // Variable que va a controlar si estamos leyendo el value de un header o un name

    char tempHeader[16];

    // Empezamos en la posicion que termino el otro parser
    for(size_t i = *position ; i < rawLength ; i++){
        
        if(rawRequestLine[i] == ':' && value == 0){

            if(i + 1 >= rawLength || rawRequestLine[i + 1] != ' ') return -1;

            size_t length = i - lastLetter;

            if(length >= sizeof(tempHeader) || length == 0) return -1;

            memcpy(tempHeader, rawRequestLine + lastLetter , length);
            tempHeader[length] = '\0';
            Request_Header_Name reqHeader = getRequestHeader(tempHeader);
            
            if(reqHeader == HEADER_UNKNOWN) return -1;

            lastLetter = i+2;
            value = 1;
        }

        if(rawRequestLine[i] == '\r' && value == 1) {
            
            if(i + 1 >= rawLength || rawRequestLine[i + 1] != '\n') return -1;

            size_t length = i - lastLetter;
            char tempValue[128];

            if(length >= sizeof(tempValue) || length == 0) return -1;

            memcpy(tempValue, rawRequestLine + lastLetter , length);
            tempValue[length] = '\0';
            
            addRequestHeader(req->headerList , tempHeader , tempValue);
            
            lastLetter = i+2;
            value = 0;
        }
        
    }

    return 0;
}