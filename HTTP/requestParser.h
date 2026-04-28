#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include "structs/request.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int parseRequestLine (const char *rawRequestLine, size_t rawLength, Request *req , size_t *position);
int parseHeaders(const char *rawRequestLine, size_t rawLength, Request *req , size_t *position);
int parseBody(const char *rawRequest, size_t rawLength , Request *req , size_t *position);

#endif
