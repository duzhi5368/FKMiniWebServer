#ifndef FWS_MiniWebServerAPI
#define FWS_MiniWebServerAPI

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "FWS_StructsAndEnums.h"

extern FILE *fpLog;

#ifdef __cplusplus
extern "C" {
#endif

void					mwInitParam(HttpParam* hp);
int						mwServerStart(HttpParam* hp);
void*					mwHttpLoop(void* _hp);
int						mwServerShutdown(HttpParam* hp);
int						mwSetRcvBufSize(WORD wSize);
PFNPOSTCALLBACK			mwPostRegister(HttpParam *httpParam, PFNPOSTCALLBACK);
PFNFILEUPLOADCALLBACK	mwFileUploadRegister(HttpParam *httpParam, PFNFILEUPLOADCALLBACK);

int						mwGetHttpDateTime(time_t tm, char *buf, int bufsize);
int						mwGetLocalFileName(HttpFilePath* hfp);
char*					mwGetVarValue(HttpVariables* vars, const char *varname, const char *defval);
int						mwGetVarValueInt(HttpVariables* vars, const char *varname, int defval);
unsigned int			mwGetVarValueHex(HttpVariables* vars, const char *varname, unsigned int defval);
int						mwParseQueryString(UrlHandlerParam* up);
int						mwGetContentType(const char *pchExtname);
void					mwDecodeString(char* s);

#ifdef __cplusplus
}
#endif

#endif //FWS_MiniWebServerAPI
