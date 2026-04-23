#ifndef GET_ENUM_REQUEST_LINE_H
#define GET_ENUM_REQUEST_LINE_H

#include "../structs/requestLine.h"

HTTP_Method getHTTPMethod(const char *strMethod);
HTTP_Version getHTTPVersion(const char *strVersion);

#endif