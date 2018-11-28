#ifndef FWS_StructsAndEnums
#define FWS_StructsAndEnums

#include "FWS_LogicMagicDefine.h"
#include "FWS_CommonDefine.h"
#include "FWS_PlatformDefine.h"

typedef int(*PFNIDLECALLBACK)(void* hp);

typedef enum {
	HTTPFILETYPE_UNKNOWN = 0,
	HTTPFILETYPE_HTML,
	HTTPFILETYPE_XML,
	HTTPFILETYPE_TEXT,
	HTTPFILETYPE_XUL,
	HTTPFILETYPE_CSS,
	HTTPFILETYPE_JS,
	HTTPFILETYPE_PNG,
	HTTPFILETYPE_JPEG,
	HTTPFILETYPE_GIF,
	HTTPFILETYPE_SWF,
	HTTPFILETYPE_MPA,
	HTTPFILETYPE_MPEG,
	HTTPFILETYPE_AVI,
	HTTPFILETYPE_MP4,
	HTTPFILETYPE_MOV,
	HTTPFILETYPE_264,
	HTTPFILETYPE_FLV,
	HTTPFILETYPE_TS,
	HTTPFILETYPE_3GP,
	HTTPFILETYPE_ASF,
	HTTPFILETYPE_OCTET,
	HTTPFILETYPE_STREAM,
	HTTPFILETYPE_M3U8,
	HTTPFILETYPE_SDP,
	HTTPFILETYPE_HEX,
} HttpFileType;

typedef enum {
	MW_INIT = 0,
	MW_UNINIT,
	MW_PARSE_ARGS,
} MW_EVENT;

typedef int(*PFNEVENTHANDLER)(MW_EVENT msg, void* handler, void* hp);

typedef struct _tagPostParam {
	//  char* pchPageName;
	struct {
		char* pchParamName;
		char* pchParamValue;
	} stParams[MAXPOSTPARAMS];
	void *httpParam;
	int iNumParams;
	char *pchPath;
} PostParam;

// multipart file upload post (per socket) structure
typedef struct {
	char pchBoundaryValue[80];
	OCTET oFileuploadStatus;
	size_t writeLocation;
	PostParam pp;
	char pchFilename[MAX_PATH];
	void *pxCallBackData;
} HttpMultipart;

typedef int(*PFNFILEUPLOADCALLBACK)(HttpMultipart*, OCTET*, size_t, char*);

typedef struct _tagSubstParam {
	char* pchParamName;
	char* pchParamValue;	// returned
	int iMaxValueBytes;
} SubstParam;

typedef union {
	unsigned long laddr;
	unsigned short saddr[2];
	unsigned char caddr[4];
} IPADDR;

typedef struct {
	int iHttpVer;
	size_t startByte;
	char *pucPath;
	const char *pucReferer;
	char* pucHost;
	int headerSize;
	char* pucPayload;
	size_t payloadSize;
	int iCSeq;
	const char* pucTransport;
	const char* pucAuthInfo;
} HttpRequest;

typedef struct {
	int statusCode;
	int headerBytes;
	int sentBytes;
	size_t contentLength;
	HttpFileType fileType;
} HttpResponse;

typedef struct {
	char *name;
	char *value;
} HttpVariables;

typedef struct {
	time_t startTime;
	WORD clientCount;
	WORD clientCountMax;
	size_t reqCount;
	size_t fileSentCount;
	size_t fileSentBytes;
	int varSubstCount;
	int urlProcessCount;
	int timeOutCount;
	int authFailCount;
	int fileUploadCount;
} HttpStats;

typedef struct _HttpSocket {
	SOCKET socket;
	IPADDR ipAddr;

	HttpRequest request;
	HttpResponse response;
	char *pucData;
	int bufferSize;			// the size of buffer pucData pointing to
	int dataLength;
#ifdef WINCE
	HANDLE fd;
#else
	int fd;
#endif
	unsigned int flags;
	void* handler;				// http handler function address
	void* ptr;
	time_t tmAcceptTime;
	time_t tmExpirationTime;
	DWORD dwResumeTick;
	int iRequestCount;
	char* mimeType;
	HttpMultipart* pxMP;
	char* buffer;
} HttpSocket;

typedef struct {
	void* hp;
	HttpSocket* hs;
	const char *pucRequest;
	HttpVariables* pxVars;
	int iVarCount;
	char *pucHeader;
	char *pucBuffer;
	char *pucPayload;
	int dataBytes;
	int contentBytes;
	HttpFileType fileType;
	void *p_sys;
} UrlHandlerParam;

typedef int(*PFNURLCALLBACK)(UrlHandlerParam*);

typedef struct {
	const char* pchUrlPrefix;
	PFNURLCALLBACK pfnUrlHandler;
	PFNEVENTHANDLER pfnEventHandler;
	void *p_sys;
} UrlHandler;

typedef struct {
	const char* pchUrlPrefix;
	const char* pchUsername;
	const char* pchPassword;
	const char* pchOtherInfo;
	char pchAuthString[MAX_AUTH_INFO_LEN];
} AuthHandler;

typedef struct {
	char* pchUrlPrefix;
	char pchLocalRealPath[MAX_PATH];
} VirtPathHandler;

typedef int(*PFNPOSTCALLBACK)(PostParam*);
typedef int(*PFNSUBSTCALLBACK)(SubstParam*);

typedef struct _httpParam {
	HttpSocket* hsSocketQueue;		/* socket queue*/
	int maxClients;
	int maxClientsPerIP;
	int bKillWebserver;
	int bWebserverRunning;
	unsigned int flags;
	SOCKET listenSocket;
	int httpPort;
	int socketRcvBufSize;			/* socket receive buffer size in KB */
	char pchWebPath[128];
	UrlHandler *pxUrlHandler;		/* pointer to URL handler array */
	AuthHandler *pxAuthHandler;     /* pointer to authorization handler array */
#ifndef DISABLE_VIRTUAL_PATH
	VirtPathHandler *pxVirtPathHandler;
#endif
	PFNSUBSTCALLBACK pfnSubst;
	PFNFILEUPLOADCALLBACK pfnFileUpload;
	PFNPOSTCALLBACK pfnPost;
	PFNIDLECALLBACK pfnIdleCallback;
	DWORD dwAuthenticatedNode;
	time_t tmAuthExpireTime;
	time_t tmSocketExpireTime;
	int maxDownloadSpeed;			/* maximum download speed in KB/s */
	HttpStats stats;
	u_long hlBindIP;
	void* szctx;
} HttpParam;

typedef struct {
	const char* pchRootPath;
	const char* pchHttpPath;
	char cFilePath[MAX_PATH];
	char* pchExt;
	int fTailSlash;
} HttpFilePath;

typedef struct {
	unsigned int flags;
	char *pchCurDir;
	char *pchPath;
#ifdef WIN32
	HANDLE fdRead;
	HANDLE fdWrite;
	int fdStdErr;
	PROCESS_INFORMATION piProcInfo;
	HWND hWnd;
#else
	int fdRead;
	int fdWrite;
	int pid;
#endif
	char *pchEnv;
	char *buffer;
	int iRetCode;
	int iBufferSize;
	int locked;
}SHELL_PARAM;

typedef struct {
	char* name;
	char* fmt;
	void* value;
	int indent;
	int flags;
} HTTP_XML_NODE;

typedef struct {
	int state;
	pthread_t thread;
	char result[16];
} HANDLER_DATA;

#endif // FWS_StructsAndEnums
