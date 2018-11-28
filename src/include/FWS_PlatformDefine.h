#ifndef FWS_PlatformDefine
#define FWS_PlatformDefine
//------------------------------------------------------------------------------
#ifdef SYS_MINGW
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#ifdef HAVE_PTHREAD
	#include <pthread.h>
#endif

#ifdef WINCE
	#include <windows.h>
	#include <winsock2.h>
	#define snprintf _snprintf

	#define ssize_t size_t
	#define socklen_t int
	#define open _open
	#define read _read
	#define write _write
	#define close _close
	#define atoll _atoi64

	#ifndef HAVE_PTHREAD
		typedef HANDLE pthread_t;
		typedef HANDLE pthread_mutex_t;
	#endif

	typedef DWORD(WINAPI *PFNGetProcessId)(HANDLE hProcess);
	#define msleep(ms) (Sleep(ms))

#elif defined(WIN32)
	#include <windows.h>
	#include <io.h>
	#define snprintf _snprintf

	#define ssize_t size_t
	#define socklen_t int
	#define open _open
	#define read _read
	#define write _write
	#define close _close
	#define atoll _atoi64

	#ifndef HAVE_PTHREAD
		typedef HANDLE pthread_t;
		typedef HANDLE pthread_mutex_t;
	#endif

	typedef DWORD(WINAPI *PFNGetProcessId)(HANDLE hProcess);
	#define msleep(ms) (Sleep(ms))

#else
	#include <stdlib.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/select.h>
	#include <netdb.h>
	#include <stdint.h>

	#if !defined(O_BINARY)
		#define O_BINARY 0
	#endif

	#define closesocket close

	#ifndef MAX_PATH
		#define MAX_PATH 256
	#endif
	#ifndef FALSE
		#define FALSE 0
	#endif
	#ifndef TRUE
		#define TRUE 1
	#endif

	typedef int SOCKET;
	typedef unsigned int DWORD;
	typedef unsigned short int WORD;
	typedef unsigned char BYTE;
	typedef int BOOL;

	#define msleep(ms) (usleep(ms<<10))
#endif

#ifndef WIN32
	#include <sys/time.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <unistd.h>
#endif

#ifndef WINCE
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <ctype.h>
	#include <fcntl.h>
#endif

#ifdef _DEBUG
	#define DBG printf
#else
	#define DBG
#endif

#ifdef WIN32
	#ifndef SYS_MINGW
		#define read _read
		#define open _open
		#define close _close
		#define lseek _lseek
		#define read _read
		#define write _write
		#define strdup _strdup
		#define dup2 _dup2
		#define dup _dup
		#define pipe _pipe
		#define spawnvpe _spawnvpe
		#define spawnvp _spawnvp
	#endif
#endif

#ifndef WINCE
	#define SLASH '/'
#else
	#define SLASH '\\'
#endif

#ifdef _7Z
	#include "7zDec/7zInc.h"
#endif

#ifdef WIN32
	#define OPEN_FLAG O_RDONLY|0x8000
#else
	#define OPEN_FLAG O_RDONLY
#endif

//------------------------------------------------------------------------------
#endif // FWS_PlatformDefine
