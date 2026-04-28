#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include "structs/requestLine.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int parseRequestLine (const char *rawRequestLine, size_t rawLength, RequestLine *req);

#endif
