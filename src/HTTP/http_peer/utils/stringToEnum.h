#ifndef STRING_TO_ENUM_H
#define STRING_TO_ENUM_H

#include "../../structs/request.h"
#include "../../structs/response.h"

HTTP_Method getHTTPMethod(const char *strMethod);
HTTP_Version getHTTPVersion(const char *strVersion);
Request_Header_Name getRequestHeader(const char *strHeader);

#endif