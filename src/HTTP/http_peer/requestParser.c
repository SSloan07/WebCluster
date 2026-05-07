#include "requestParser.h"
#include "utils/stringToEnum.h"
#include "utils/enumToString.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#define MAX_URI_LENGTH 2048

int parseRequestLine(const char *rawRequestLine , size_t rawLength, Request *req , size_t *position){

    int count = 0;
    size_t lastLetter = 0;

    // Parse all blank spaces and determine which separator belongs to each part of the request
    for(size_t i = 0 ; i < rawLength ; i++){
        
        // Move the pointer i+1 times (since i is the separator between expressions)
        // rawRequestLine + lastLetter moves the pointer lastLetter positions so it points to the last character found
        if(rawRequestLine[i] == ' ' && count == 0){

            size_t length = i - lastLetter;
            char tempMethod[16];

            if(length >= sizeof(tempMethod) || length == 0) return -1;

            memcpy(tempMethod, rawRequestLine + lastLetter , length); // memcpy(destination, source, number of bytes to copy)
            tempMethod[length] = '\0';
            req->method = getHTTPMethod(tempMethod);
            
            lastLetter = i+1;
            count++;
        }
        
        else if(rawRequestLine[i] == ' ' && count == 1){

            size_t length = i - lastLetter;

            if (length == 0 || length > MAX_URI_LENGTH) return -1;

            req->requestURI = malloc(length + 1);
            if (req->requestURI == NULL) return -1;

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
            
            *position = i + 2;
            count++;
        }
    }

    if(count != 3){
        printf("No se encontraron los 3 argumentos necesarios del request line\n");
        return -1;
    }

    return 0;
}

int parseHeaders(const char *rawRequestLine, size_t rawLength, Request *req, size_t *position) {

    size_t lastLetter = *position;
    int value = 0;

    char tempHeader[128];
    char tempValue[1024];
    Request_Header_Name reqHeader = HEADER_UNKNOWN;

    int foundEnd = 0;

    for (size_t i = *position; i < rawLength; i++) {

        if (rawRequestLine[i] == ':' && value == 0) {

            size_t length = i - lastLetter;
            if (length >= sizeof(tempHeader) || length == 0) return -1;

            memcpy(tempHeader, rawRequestLine + lastLetter, length);
            tempHeader[length] = '\0';
            reqHeader = getRequestHeader(tempHeader);

            lastLetter = i + 1;
            while (lastLetter < rawLength && rawRequestLine[lastLetter] == ' ') {
                lastLetter++;
            }
            value = 1;
        }

        if (rawRequestLine[i] == '\r' && value == 1) {
            if (i + 1 >= rawLength || rawRequestLine[i + 1] != '\n') return -1;

            size_t length = i - lastLetter;
            if (length >= sizeof(tempValue)) return -1;

            memcpy(tempValue, rawRequestLine + lastLetter, length);
            tempValue[length] = '\0';

            if (addRequestHeader(req->headerList, reqHeader, tempHeader, tempValue) != 0) {
                return -1;
            }

            lastLetter = i + 2;
            value = 0;

            if (i + 3 < rawLength && rawRequestLine[i + 2] == '\r' && rawRequestLine[i + 3] == '\n') {
                *position = i + 4;
                foundEnd = 1;
                break;
            }
        }
    }

    if (!foundEnd) return -1;

    return 0;
}

int parseBody(const char *rawRequest, size_t rawLength , Request *req , size_t *position) {

    if(req->method != METHOD_POST && req->method != METHOD_PUT) return -1; // Only POST and PUT handle a body here

    if(*position >= rawLength) return -1; // There is no body

    Request_Header *headerLength = searchHeader(req->headerList , HEADER_CONTENT_LENGTH);

    if(headerLength == NULL) return -1; // The header does not exist
    
    // Convert from char to size_t to know how much we need to read
    char *end;
    errno = 0; // Good practice

    unsigned long value = strtoul(headerLength->value , &end , 10);

    if(end == headerLength->value) return -1; // If both point to the same address, nothing was converted
    if(errno != 0) return -1;

    size_t bodyLength = (size_t)value;
    // -----------------------------

    if(bodyLength == 0) return 0;
    if( (rawLength - *position) < bodyLength) return -1; // The provided Content-Length is incorrect

    req->body = malloc(bodyLength);
    if (req->body == NULL) return -1;

    memcpy(req->body , rawRequest + *position, bodyLength);
    req->bodyLength = bodyLength;

    return 0;
}
