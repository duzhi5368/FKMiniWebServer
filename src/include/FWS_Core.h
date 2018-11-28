#ifndef FWS_Core
#define FWS_Core

#include "FWS_PlatformDefine.h"
#include "FWS_StructsAndEnums.h"

#pragma warning( disable : 4996)

void	_mwInitSocketData(HttpSocket *phsSocket);
int		_mwGetConnFromIP(HttpParam* hp, IPADDR ip);
SOCKET	_mwAcceptSocket(HttpParam* hp, struct sockaddr_in *sinaddr);
void	_mwDenySocket(HttpParam* hp, struct sockaddr_in *sinaddr);
int		_mwProcessReadSocket(HttpParam* hp, HttpSocket* phsSocket);
int		_mwProcessWriteSocket(HttpParam *hp, HttpSocket* phsSocket);
void	_mwCloseSocket(HttpParam* hp, HttpSocket* phsSocket);
int		_mwStartSendFile(HttpParam* hp, HttpSocket* phsSocket);
int		_mwSendFileChunk(HttpParam *hp, HttpSocket* phsSocket);
void	_mwProcessPost(HttpParam* httpParam, HttpSocket* phsSocket);
int		_mwProcessMultipartPost(HttpParam *httpParam, HttpSocket* phsSocket, BOOL fNoRecv);
int		_mwSubstVariables(HttpParam* hp, char* pchData, int iLength, int* piBytesUsed);
char*	_mwStrStrNoCase(char* pchHaystack, char* pchNeedle);
void	_mwProcessPostVars(HttpParam *httpParam, HttpSocket* phsSocket, int iContentOffset, int contentLength);
void	_mwRedirect(HttpSocket* phsSocket, char* pchFilename);
int		_mwSendRawDataChunk(HttpParam *hp, HttpSocket* phsSocket);
int		_mwStartSendRawData(HttpParam *hp, HttpSocket* phsSocket);
int		_mwGetToken(char* pchBuffer, int iTokenNumber, char** ppchToken);
char	_mwDecodeCharacter(char* pchEncodedChar);
int		_mwLoadFileChunk(HttpParam *hp, HttpSocket* phsSocket);
char*	_mwFindMultipartBoundary(char *poHaystack, int iHaystackSize, char *poNeedle);
void	_mwNotifyPostVars(HttpParam *hp, HttpSocket* phsSocket, PostParam *pp);
BOOL	_mwCheckAuthentication(HttpParam *hp, HttpSocket* phsSocket);
int		_mwStartSendMemoryData(HttpSocket* phsSocket);
int		_GetContentType(char *pchFilename);
int		_mwCheckAccess(HttpSocket* phsSocket);
int		_mwGetContentType(char *pchExtname);
int		_mwSendHttpHeader(HttpSocket* phsSocket);
char*	_mwStrDword(char* pchHaystack, DWORD dwSub, DWORD dwCharMask);
SOCKET	_mwStartListening(HttpParam* hp);
int		_mwParseHttpHeader(HttpSocket* phsSocket);
int		_mwStrCopy(char *dest, const char *src);
int		_mwStrHeadMatch(char** pbuf1, const char* buf2);
int		_mwRemoveSocket(HttpParam* hp, HttpSocket* hs);
void	_mwSendErrorPage(SOCKET socket, const char* header, const char* body);
void	_mwCloseAllConnections(HttpParam* hp);

#endif