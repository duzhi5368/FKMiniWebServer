#ifndef FWS_PlatformIndependentLayer
#define FWS_PlatformIndependentLayer
//------------------------------------------------------------------------------
#pragma warning( disable : 4996)

#include "FWS_PlatformDefine.h"
#include "FWS_StructsAndEnums.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

	char*			GetTimeString();
	char*			GetAppName(const char* commandLine);
	char**			Tokenize(char* str, char delimiter);
	void			GetFullPath(char* buffer, char* argv0, char* path);
	char*			GetLocalAddrString();

	int				InitSocket();
	void			UninitSocket();

	int				ThreadCreate(pthread_t *pth, void* (*start_routine)(void*), void* arg);
	int				ThreadKill(pthread_t pth);
	int				ThreadWait(pthread_t pth, int timeout, void** ret);

	void			MutexCreate(pthread_mutex_t* mutex);
	void			MutexDestroy(pthread_mutex_t* mutex);
	void			MutexLock(pthread_mutex_t* mutex);
	void			MutexUnlock(pthread_mutex_t* mutex);

	int				ReadDir(const char* pchDir, char* pchFileNameBuf);
	int				IsFileExist(const char* filename);
	int				IsDir(const char* pchName);

	int				ShellRead(SHELL_PARAM* param, int timeout);
	int				ShellWrite(SHELL_PARAM* param, void* data, int bytes);
	void			ShellClean(SHELL_PARAM* param);
	int				ShellWait(SHELL_PARAM* param, int iTimeout);
	int				ShellExec(SHELL_PARAM* param, const char* cmdline);
	int				ShellTerminate(SHELL_PARAM* param);
	int				ShellRun(SHELL_PARAM* param, const char* cmdline);

	int				WriteXmlHeader(char** pbuf, int* pbufsize, int ver, char* charset, char* xsl);
	int				WriteXmlLine(char** pbuf, int* pbufsize, HTTP_XML_NODE *node, char *attr);
	void			WriteXmlString(char** pbuf, int* pbufsize, int indent, char* str);
#ifndef WIN32
	unsigned int	GetTickCount();
#endif


#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif // FWS_PlatformIndependentLayer
