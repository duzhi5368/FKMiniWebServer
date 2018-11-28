#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "../include/FWS_PlatformIndependentLayer.h"
//------------------------------------------------------------------------------
int	InitSocket() {
#ifdef WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) &&
		WSAStartup(MAKEWORD(1, 1), &wsaData)) {
		return 0;
	}
#endif
	return 1;
}
//------------------------------------------------------------------------------
void UninitSocket() {
#ifdef WIN32
	WSACleanup();
#endif
}
//------------------------------------------------------------------------------
char *GetTimeString() {
	static char buf[16];
	memset(buf, 0, sizeof(char) * 16);
#ifndef WINCE
	time_t tm = time(NULL);
	memcpy(buf, ctime(&tm) + 4, 15);
	buf[15] = 0;
#endif
	return buf;
}
//------------------------------------------------------------------------------
int ThreadCreate(pthread_t *pth, void* (*start_routine)(void*), void* arg) {
#ifndef HAVE_PTHREAD
	DWORD dwid;
	*pth = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)start_routine, arg, 0, &dwid);
	return *pth != NULL ? 0 : 1;
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	int ret = pthread_create(pth, &attr, start_routine, arg);
	pthread_attr_destroy(&attr);
	return ret;
#endif
}
//------------------------------------------------------------------------------
int ThreadKill(pthread_t pth) {
#ifndef HAVE_PTHREAD
	return TerminateThread(pth, 0) ? 0 : 1;
#else
	return pthread_cancel(pth);
#endif
}
//------------------------------------------------------------------------------
int ThreadWait(pthread_t pth, int timeout, void** ret) {
#ifndef HAVE_PTHREAD
	if (WaitForSingleObject(pth, timeout) == WAIT_TIMEOUT)
		return 1;
	if (ret) GetExitCodeThread(pth, (LPDWORD)ret);
	CloseHandle(pth);
	return 0;
#else
	return pthread_join(pth, ret);
#endif
}
//------------------------------------------------------------------------------
void MutexCreate(pthread_mutex_t* mutex){
#ifndef HAVE_PTHREAD
	*mutex = CreateMutex(0, FALSE, NULL);
#else
	pthread_mutex_init(mutex, NULL);
#endif
}
//------------------------------------------------------------------------------
void MutexDestroy(pthread_mutex_t* mutex) {
#ifndef HAVE_PTHREAD
	CloseHandle(*mutex);
#else
	pthread_mutex_destroy(mutex);
#endif
}
//------------------------------------------------------------------------------
void MutexLock(pthread_mutex_t* mutex) {
#ifndef HAVE_PTHREAD
	WaitForSingleObject(*mutex, INFINITE);
#else
	pthread_mutex_lock(mutex);
#endif
}
//------------------------------------------------------------------------------
void MutexUnlock(pthread_mutex_t* mutex) {
#ifndef HAVE_PTHREAD
	ReleaseMutex(*mutex);
#else
	pthread_mutex_unlock(mutex);
#endif
}
//------------------------------------------------------------------------------
int ReadDir(const char* pchDir, char* pchFileNameBuf) {
#if defined(WIN32) || defined(WINCE)
	static HANDLE hFind = NULL;
	WIN32_FIND_DATA finddata;

	if (!pchFileNameBuf) {
		if (hFind) {
			FindClose(hFind);
			hFind = NULL;
		}
		return 0;
	}
	if (pchDir) {
		char *p;
		int len;
		if (!IsDir(pchDir)) return -1;
		if (hFind) FindClose(hFind);
		len = strlen(pchDir);
		p = (char *)(malloc(len + 5));
		snprintf(p, len + 5, "%s\\*.*", pchDir);
		hFind = FindFirstFile(p, &finddata);
		free(p);
		if (hFind == INVALID_HANDLE_VALUE) {
			hFind = NULL;
			return -1;
		}
		strcpy(pchFileNameBuf, finddata.cFileName);
		return 0;
	}
	if (!hFind) return -1;
	if (!FindNextFile(hFind, &finddata)) {
		FindClose(hFind);
		hFind = NULL;
		return -1;
	}
	strcpy(pchFileNameBuf, finddata.cFileName);
#else
	static DIR *stDirIn = NULL;
	struct dirent *stFiles;

	if (!pchFileNameBuf) {
		if (stDirIn) {
			closedir(stDirIn);
			stDirIn = NULL;
		}
		return 0;
	}
	if (pchDir) {
		if (!IsDir(pchDir)) return -1;
		if (stDirIn) closedir(stDirIn);
		stDirIn = opendir(pchDir);
	}
	if (!stDirIn) return -1;
	stFiles = readdir(stDirIn);
	if (!stFiles) {
		closedir(stDirIn);
		stDirIn = NULL;
		return -1;
	}
	strcpy(pchFileNameBuf, stFiles->d_name);
#endif
	return 0;
}
//------------------------------------------------------------------------------
int IsFileExist(const char* filename) {
#ifdef WINCE
	WIN32_FIND_DATA f;
	HANDLE hFind = FindFirstFile(filename, &f);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;
	FindClose(hFind);
	return 1;
#else
	struct stat stat_ret;
	if (stat(filename, &stat_ret) != 0) return 0;

	return (stat_ret.st_mode & S_IFREG) != 0;
#endif
}
//------------------------------------------------------------------------------
int IsDir(const char* pchName) {
	struct stat stDirInfo;
	if (stat(pchName, &stDirInfo) < 0) return 0;
	return (stDirInfo.st_mode & S_IFDIR) ? 1 : 0;
}
//------------------------------------------------------------------------------
#ifndef WIN32
unsigned int GetTickCount()
{
	struct timeval ts;
	gettimeofday(&ts, 0);
	return ts.tv_sec * 1000 + ts.tv_usec / 1000;
}
#endif
//------------------------------------------------------------------------------
int ShellRead(SHELL_PARAM* param, int timeout)
{
#ifndef WIN32
	int ret;
	fd_set fds;
	struct timeval tv;
#endif

	if (!param->buffer || param->iBufferSize <= 0) {
		if (param->flags & SF_ALLOC) {
			param->buffer = (char*)(malloc(SF_BUFFER_SIZE));
			param->iBufferSize = SF_BUFFER_SIZE;
		}
		else {
			return -1;
		}
	}

#ifndef WIN32
	FD_ZERO(&fds);
	FD_SET(param->fdRead, &fds);
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
#endif

	if (!(param->flags & SF_READ_STDOUT_ALL)) {
#ifdef WIN32
		DWORD bytes;
		BOOL success;
		DWORD start = GetTickCount();
		do {
			if (!PeekNamedPipe(param->fdRead, 0, 0, 0, &bytes, 0)) return -1;
		} while (bytes == 0 && WaitForSingleObject(param->piProcInfo.hProcess, 50) == WAIT_TIMEOUT && GetTickCount() - start < (DWORD)timeout);
		if (bytes == 0) return 0;

		param->locked++;
		success = ReadFile(param->fdRead, param->buffer, param->iBufferSize - 1, &bytes, NULL);
		param->locked--;
		if (!success)
			return -1;
		param->buffer[bytes] = 0;
		return bytes;
#else
		if (select(param->fdRead + 1, &fds, NULL, NULL, &tv) < 1) return -1;
		ret = read(param->fdRead, param->buffer, param->iBufferSize - 1);
		if (ret<1) return -1;
		param->buffer[ret] = 0;
		return ret;
#endif
	}
	else {
#ifdef WIN32
		size_t offset = 0;
		int fSuccess;
		for (;;) {
			DWORD bytes;
			DWORD start = GetTickCount();
			do {
				if (!PeekNamedPipe(param->fdRead, 0, 0, 0, &bytes, 0)) return -1;
			} while (bytes == 0 && WaitForSingleObject(param->piProcInfo.hProcess, 50) == WAIT_TIMEOUT && GetTickCount() - start < (DWORD)timeout);
			if (bytes == 0) return 0;

			if (offset + bytes + 1 >= (size_t)param->iBufferSize) {
				if (param->flags & SF_ALLOC) {
					param->iBufferSize = max(param->iBufferSize * 2, (int)(offset + bytes + 1));
					param->buffer = (char*)(realloc(param->buffer, param->iBufferSize));
				}
				else {
					break;
				}
			}
			param->locked++;
			fSuccess = ReadFile(param->fdRead, param->buffer + offset, param->iBufferSize - 1 - offset, &bytes, NULL);
			param->locked--;
			if (!fSuccess) break;
			offset += bytes;
		}
		param->buffer[offset] = 0;
		return offset;
#endif
	}
}
//------------------------------------------------------------------------------
int ShellWrite(SHELL_PARAM* param, void* data, int bytes)
{
#ifdef WIN32
	DWORD written;
	return WriteFile(param->fdWrite, data, bytes, &written, 0) ? written : -1;
#else
	return write(param->fdWrite, data, bytes);
#endif
}
//------------------------------------------------------------------------------
int ShellTerminate(SHELL_PARAM* param)
{
	int ret;
#ifdef WIN32
	if (!param->piProcInfo.hProcess) return -1;
	ret = TerminateProcess(param->piProcInfo.hProcess, 0) ? 0 : -1;
#else
	if (!param->pid) return 0;
	ret = kill(param->pid, SIGKILL);
	param->pid = 0;
#endif
	return ret;
}
//------------------------------------------------------------------------------
void ShellClean(SHELL_PARAM* param)
{
#ifdef WIN32
	while (param->locked) Sleep(10);
	if (param->fdRead) CloseHandle((HANDLE)param->fdRead);
	if (param->fdWrite) CloseHandle((HANDLE)param->fdWrite);
	if (param->piProcInfo.hProcess) CloseHandle(param->piProcInfo.hProcess);
	if (param->piProcInfo.hThread) CloseHandle(param->piProcInfo.hThread);
	memset(&param->piProcInfo, 0, sizeof(PROCESS_INFORMATION));
#else
	if (param->fdRead) close(param->fdRead);
	if (param->fdWrite) close(param->fdWrite);
	param->pid = 0;
#endif
	param->fdRead = 0;
	param->fdWrite = 0;
	if (param->flags & SF_ALLOC) {
		free(param->buffer);
		param->buffer = NULL;
		param->iBufferSize = 0;
	}
}
//------------------------------------------------------------------------------
int ShellWait(SHELL_PARAM* param, int iTimeout)
{
#ifdef WIN32
	switch (WaitForSingleObject(param->piProcInfo.hProcess, (iTimeout == -1) ? INFINITE : iTimeout)) {
	case WAIT_OBJECT_0:
		GetExitCodeProcess(param->piProcInfo.hProcess, (LPDWORD)(&param->iRetCode));
		return 1;
	case WAIT_TIMEOUT:
		return 0;
	default:
		return -1;
	}
#else
	int ret = -1;
	return waitpid(param->pid, &ret, iTimeout == 0 ? WNOHANG : 0);
#endif
}
//------------------------------------------------------------------------------
char* GetAppName(const char* commandLine)
{
	char* appname;
	char* p;

	p = (char*)(strrchr(commandLine, '.'));
	if (p && !_stricmp(p + 1, "bat")) {
		return _strdup("cmd.exe");
	}
	else if (*commandLine == '\"') {
		appname = _strdup(commandLine + 1);
		p = strchr(appname, '\"');
		*p = 0;
	}
	else {
		p = (char*)(strchr(commandLine, ' '));
		if (p) {
			int l = p - commandLine;
			appname = (char*)(malloc(l + 5));
			strncpy(appname, commandLine, l);
			appname[l] = 0;
			if (!strchr(appname, '.')) strcat(appname, ".exe");
		}
		else {
			appname = _strdup(commandLine);
		}
	}
	return appname;
}
//------------------------------------------------------------------------------
char** Tokenize(char* str, char delimiter)
{
	char** tokens;
	int n = 1;
	int i;
	char *p;

	// find out number of tokens
	p = str;
	for (;;) {
		while (*p && *p != delimiter) p++;
		if (!*p) break;
		n++;
		while (*(++p) == delimiter);
	}
	// allocate buffer for array
	tokens = (char**)calloc(n + 1, sizeof(char*));
	// store pointers to tokens
	p = str;
	for (i = 0; i < n; i++) {
		if (*p == '\"') {
			tokens[i] = ++p;
			while (*p && *p != '\"') p++;
			if (!*p) {
				i++;
				break;
			}
		}
		else {
			tokens[i] = p;
			while (*p && *p != delimiter) p++;
			if (!*p) {
				i++;
				break;
			}
		}
		*p = 0;
		while (*(++p) == delimiter);
	}
	//tokens[i] = "";
	return tokens;
}
//------------------------------------------------------------------------------
void GetFullPath(char* buffer, char* argv0, char* path)
{
	char* p = strrchr(argv0, '/');
	if (!p) p = strrchr(argv0, '\\');
	if (!p) {
		strcpy(buffer, path);
	}
	else {
		int l = p - argv0 + 1;
		memcpy(buffer, argv0, l);
		strcpy(buffer + l, path);
	}
}
//------------------------------------------------------------------------------
char* GetLocalAddrString()
{
	// get local ip address
	struct sockaddr_in sock;
	char hostname[128];
	struct hostent * lpHost;
	gethostname(hostname, 128);
	lpHost = gethostbyname(hostname);
	memcpy(&(sock.sin_addr), (void*)lpHost->h_addr_list[0], lpHost->h_length);
	return inet_ntoa(sock.sin_addr);
}
//------------------------------------------------------------------------------
int ShellExec(SHELL_PARAM* param, const char* cmdline)
{
#ifdef WIN32
	SECURITY_ATTRIBUTES saAttr;
	STARTUPINFO siStartInfo;
	BOOL fSuccess;
	char newPath[256], prevPath[256];
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr;
#else
	int fdin[2], fdout[2], pid;
	int fdStdinChild;
	int fdStdoutChild;
	char *filePath;
	char *prevPath;
	char *env[2];

	char *_cmdline = strdup(cmdline);
	char **argv = Tokenize(_cmdline, ' ');
#endif

#ifdef WIN32
	if (param->piProcInfo.hProcess || !cmdline) 
		return -1;

	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	// modify path variable
	if (param->pchPath) {
		GetEnvironmentVariable("PATH", prevPath, sizeof(prevPath));
		snprintf(newPath, sizeof(newPath), "%s;%s", param->pchPath, prevPath);
		SetEnvironmentVariable("PATH", newPath);
	}

	memset(&param->piProcInfo, 0, sizeof(PROCESS_INFORMATION));
	memset(&siStartInfo, 0, sizeof(STARTUPINFO));

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (param->flags & (SF_REDIRECT_STDOUT | SF_REDIRECT_STDERR)) {
		// Create a pipe for the child process's STDOUT.
		if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
			return -1;
		}
		// Create noninheritable read handle and close the inheritable read
		// handle.
		fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
			GetCurrentProcess(), (LPHANDLE)&param->fdRead, 0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		if (!fSuccess) return 0;
		CloseHandle(hChildStdoutRd);
		if (param->flags & SF_REDIRECT_STDOUT) {
			siStartInfo.hStdOutput = hChildStdoutWr;
		}
		if (param->flags & SF_REDIRECT_STDERR) {
			siStartInfo.hStdError = hChildStdoutWr;
		}
	}
	siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	if (param->flags & SF_REDIRECT_STDIN) {
		// Create a pipe for the child process's STDIN.
		if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) return 0;

		// Duplicate the write handle to the pipe so it is not inherited.
		fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
			GetCurrentProcess(), (LPHANDLE)&param->fdWrite, 0,
			FALSE,                  // not inherited
			DUPLICATE_SAME_ACCESS);
		if (!fSuccess) return -1;
		CloseHandle(hChildStdinWr);
		siStartInfo.hStdInput = hChildStdinRd;
	}

	siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = (param->flags & SF_SHOW_WINDOW) ? SW_SHOW : SW_HIDE;
	if (param->flags & (SF_REDIRECT_STDIN | SF_REDIRECT_STDOUT | SF_REDIRECT_STDERR)) siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// child process
	{
		char* appname = GetAppName(cmdline);
		fSuccess = CreateProcess(0,
			(char*)cmdline,	// command line
			NULL,					// process security attributes
			NULL,					// primary thread security attributes
			TRUE,					// handles are inherited
			0,						// creation flags
			NULL,					// process' environment
			param->pchCurDir,		// current directory
			&siStartInfo,			// STARTUPINFO pointer
			&param->piProcInfo);	// receives PROCESS_INFORMATION

		free(appname);
	}

	if (param->pchPath) SetEnvironmentVariable("PATH", prevPath);
	if (!fSuccess) return -1;
	WaitForInputIdle(param->piProcInfo.hProcess, INFINITE);

	if (param->flags & SF_REDIRECT_STDIN)
		CloseHandle(hChildStdinRd);
	if (param->flags & SF_REDIRECT_STDOUT)
		CloseHandle(hChildStdoutWr);
#else

	if (param->pid) return -1;

	if (param->flags & SF_REDIRECT_STDIN) {
		pipe(fdin);
		param->fdWrite = fdin[1];
		fdStdinChild = fdin[0];
	}
	if (param->flags & SF_REDIRECT_STDOUT) {
		pipe(fdout);
		param->fdRead = fdout[0];
		fdStdoutChild = fdout[1];
	}

	pid = fork();
	if (pid == -1) return -1;
	if (pid == 0) { /* chid process */
		int i;
		//generate argument list
#ifdef _DEBUG
		printf("cmdline:%s\n", cmdline);
#endif
		//TODO: Tokenize() is not good
		filePath = argv[0];

		//set PATH
		env[0] = NULL;
		if (param->pchPath) {
			prevPath = getenv("PATH");
			env[0] = malloc(strlen(prevPath) + strlen(param->pchPath) + 2 + 5);
			sprintf(env[0], "PATH=%s:%s", prevPath, param->pchPath);
			env[1] = NULL;
		}
		if (param->flags & SF_REDIRECT_STDIN) {
			close(fdin[1]);
			dup2(fdStdinChild, 0);
		}
		if (param->flags & SF_REDIRECT_STDOUT) {
			close(fdout[0]);
			dup2(fdStdoutChild, 1);
		}
		for (i = 0; env[i] != NULL; i++) {
			printf("env[%d]=%s\n", i, env[i]);
		}
		if (execve(filePath, argv, env)<0) {
			printf("Error starting specified program\n");
		}
		return 0;
	}
	if (param->flags & SF_REDIRECT_STDIN) {
		close(fdin[0]);
	}
	if (param->flags & SF_REDIRECT_STDOUT) {
		close(fdout[1]);
	}
	param->pid = pid;
	free(_cmdline);
	free(argv);
#endif
	return 0;
}
//------------------------------------------------------------------------------
int ShellRun(SHELL_PARAM* param, const char* cmdline)
{
	int ret;
	SHELL_PARAM* proc = param;
	if (!param) {
		proc = (SHELL_PARAM*)(calloc(1, sizeof(SHELL_PARAM)));
	}

#ifdef _DEBUG
	printf("# %s\n", cmdline);
#endif
	ret = ShellExec(proc, cmdline);
	if (ret == 0) {
		if (proc->flags & SF_READ_STDOUT_ALL)
			ShellRead(proc, -1);
		else
			ShellWait(proc, -1);
	}
	ShellClean(proc);

	if (!param) free(proc);
	return ret;
}
//------------------------------------------------------------------------------
int WriteXmlHeader(char** pbuf, int* pbufsize, int ver, char* charset, char* xsl)
{
	int len2 = 0;
	int len1 = snprintf(*pbuf, *pbufsize, "<?xml version=\"%d.%d\" encoding=\"%s\"?>\n",
		ver / 10, ver % 10, charset ? charset : "utf-8");
	*pbuf += len1;
	*pbufsize -= len1;
	if (xsl) {
		len2 = snprintf(*pbuf, *pbufsize, "<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>\n", xsl);
		*pbuf += len2;
		*pbufsize -= len2;
	}
	return len1 + len2;
}
//------------------------------------------------------------------------------
void WriteXmlString(char** pbuf, int* pbufsize, int indent, char* str)
{
	int i;
	int len;
	int bufsize = *pbufsize;
	if (bufsize < indent * 2) return;
	for (i = 0; i < indent; i++) {
		*((*pbuf)++) = ' ';
		*((*pbuf)++) = ' ';
	}
	bufsize -= indent * 2;
	len = snprintf(*pbuf, bufsize, "%s%s", str, indent ? "\n" : "");
	*pbuf += len;
	bufsize -= len;
	*pbufsize = bufsize;
}
//------------------------------------------------------------------------------
int WriteXmlLine(char** pbuf, int* pbufsize, HTTP_XML_NODE *node, char *attr)
{
	int bufsize = *pbufsize;
	int len;
	int i;

	if (bufsize < node->indent * 2) return -1;
	for (i = 0; i < node->indent; i++) {
		*((*pbuf)++) = ' ';
		*((*pbuf)++) = ' ';
	}
	bufsize -= node->indent * 2;
	len = snprintf(*pbuf, bufsize, "<%s", node->name);
	*pbuf += len;
	bufsize -= len;
	if (attr) {
		len = snprintf(*pbuf, bufsize, " %s", attr);
		*pbuf += len;
		bufsize -= len;
	}
	*((*pbuf)++) = '>';
	bufsize--;

	if (node->flags & XN_CDATA) {
		len = snprintf(*pbuf, bufsize, "%s", "<![CDATA[");
		*pbuf += len;
		bufsize -= len;
	}

	len = snprintf(*pbuf, bufsize, node->fmt, node->value);
	*pbuf += len;
	bufsize -= len;

	if (node->flags & XN_CDATA) {
		len = snprintf(*pbuf, bufsize, "%s", "]]>");
		*pbuf += len;
		bufsize -= len;
	}

	len = snprintf(*pbuf, bufsize, "</%s>\n", node->name);
	*pbuf += len;
	bufsize -= len;

	*pbufsize = bufsize;
	return 0;
}
//------------------------------------------------------------------------------