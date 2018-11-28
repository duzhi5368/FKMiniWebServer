#ifndef FWS_CommonDefine
#define FWS_CommonDefine

#ifndef min
#define min(x,y) (x>y?y:x)
#endif

typedef unsigned char OCTET;

#define SHELL_NOREDIRECT 1
#define SHELL_SHOWWINDOW 2
#define SHELL_NOWAIT 4

#define LOG_ERR 1
#define ASSERT
#define GETDWORD(ptrData) (*(DWORD*)(ptrData))
#define SETDWORD(ptrData,data) (*(DWORD*)(ptrData)=data)
#define GETWORD(ptrData) (*(WORD*)(ptrData))
#define SETWORD(ptrData,data) (*(WORD*)(ptrData)=data)

#ifndef BIG_ENDINE
#define DEFDWORD(char1,char2,char3,char4) ((char1)+((char2)<<8)+((char3)<<16)+((char4)<<24))
#define DEFWORD(char1,char2) (char1+(char2<<8))
#else
#define DEFDWORD(char1,char2,char3,char4) ((char4)+((char3)<<8)+((char2)<<16)+((char1)<<24))
#define DEFWORD(char1,char2) (char2+(char1<<8))
#endif

#define SETFLAG(hs,bit) (hs->flags|=(bit));
#define CLRFLAG(hs,bit) (hs->flags&=~(bit));
#define ISFLAGSET(hs,bit) (hs->flags&(bit))

#define LOG_INFO fpLog
#define SYSLOG fprintf

#define SF_ALLOC 0x1
#define SF_SHOW_WINDOW 0x2
#define SF_READ_STDOUT_ALL 0x4
#define SF_CONVERT_LF 0x8
#define SF_REDIRECT_STDIN 0x1000
#define SF_REDIRECT_STDOUT 0x2000
#define SF_REDIRECT_STDERR 0x4000
#define SF_REDIRECT_OUTPUT (0x8000 | SF_REDIRECT_STDOUT)
#define SF_BUFFER_SIZE 256

#define XN_CDATA 1

#endif