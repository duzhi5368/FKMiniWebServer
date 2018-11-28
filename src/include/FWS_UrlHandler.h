#ifndef FWS_UrlHandler
#define FWS_UrlHandler

#include "FWS_StructsAndEnums.h"

int Func_UrlHandlerStats(UrlHandlerParam* param);
int Func_UrlHandlerAsyncData(UrlHandlerParam* param);
#ifdef _7Z
int Func_UrlHandler7Zip(UrlHandlerParam* param);
#endif


int DefaultWebFileUploadCallback(HttpMultipart *pxMP, OCTET *poData, size_t dwDataChunkSize, char* pchWebPath);
int DefaultWebSubstCallback(SubstParam* sp);
int DefaultWebPostCallback(PostParam* pp);

#endif // FWS_UrlHandler
