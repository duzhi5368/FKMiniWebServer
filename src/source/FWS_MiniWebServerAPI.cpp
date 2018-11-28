#include "../include/FWS_MiniWebServerAPI.h"
#include "../include/FWS_BaseFunctions.h"
#include "../include/FWS_CommonDefine.h"
#include "../include/FWS_HttpDefine.h"
#include "../include/FWS_Core.h"
#include "../include/FWS_UrlHandler.h"
//------------------------------------------------------------------------------
const char *dayNames = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
const char *monthNames = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";
const char *httpDateTimeFormat = "%s, %02d %s %d %02d:%02d:%02d GMT";
//------------------------------------------------------------------------------
FILE *fpLog = NULL;
//------------------------------------------------------------------------------
AuthHandler DefaultAuthHandlerList[] = {
	{ "stats", "user", "pass", "group=admin", "" },
	{ NULL }
};
//------------------------------------------------------------------------------
char* mwGetVarValue(HttpVariables* vars, const char *varname, const char *defval)
{
	int i;
	if (vars && varname) {
		for (i = 0; (vars + i)->name; i++) {
			if (!strcmp((vars + i)->name, varname)) {
				return (vars + i)->value;
			}
		}
	}
	return (char*)defval;
}
//------------------------------------------------------------------------------
int mwGetVarValueInt(HttpVariables* vars, const char *varname, int defval)
{
	int i;
	if (vars && varname) {
		for (i = 0; (vars + i)->name; i++) {
			if (!strcmp((vars + i)->name, varname)) {
				char *p = (vars + i)->value;
				return p ? atoi(p) : defval;
			}
		}
	}
	return defval;
}
//------------------------------------------------------------------------------
unsigned int mwGetVarValueHex(HttpVariables* vars, const char *varname, unsigned int defval)
{
	int i;
	if (vars && varname) {
		for (i = 0; (vars + i)->name; i++) {
			if (!strcmp((vars + i)->name, varname)) {
				char *p = (vars + i)->value;
				return p ? hex2uint32(p) : defval;
			}
		}
	}
	return defval;
}
//------------------------------------------------------------------------------
int mwGetHttpDateTime(time_t timer, char *buf, int bufsize)
{
#ifndef WINCE
	struct tm *btm;
	btm = gmtime(&timer);
	return snprintf(buf, bufsize, httpDateTimeFormat,
		dayNames + (btm->tm_wday << 2),
		btm->tm_mday,
		monthNames + (btm->tm_mon << 2),
		1900 + btm->tm_year,
		btm->tm_hour,
		btm->tm_min,
		btm->tm_sec);
#else
	return 0;
#endif
}
//------------------------------------------------------------------------------
void mwInitParam(HttpParam* hp)
{
	memset(hp, 0, sizeof(HttpParam));
	hp->httpPort = 80;
	hp->tmSocketExpireTime = 60;
	hp->maxClients = HTTP_MAX_CLIENTS;
	hp->maxClientsPerIP = HTTP_MAX_CLIENTS_PER_IP;
	hp->flags = FLAG_DIR_LISTING;
	hp->pxAuthHandler = DefaultAuthHandlerList;
	hp->pfnPost = DefaultWebPostCallback;
	hp->pfnFileUpload = DefaultWebFileUploadCallback;
}
//------------------------------------------------------------------------------
int mwServerStart(HttpParam* hp)
{
	if (hp->bWebserverRunning) {
		DBG("Error: Webserver thread already running\n");
		return -1;
	}

	if (!fpLog) {
		fpLog = stderr;
	}

	if (hp->maxClients == 0) {
		SYSLOG(LOG_INFO, "Maximum clients not set\n");
		return -1;
	}
	hp->hsSocketQueue = (HttpSocket*)(calloc(hp->maxClients, sizeof(HttpSocket)));

	if (hp->pxUrlHandler) {
		int i;
		for (i = 0; hp->pxUrlHandler[i].pchUrlPrefix; i++) {
			if (hp->pxUrlHandler[i].pfnEventHandler &&
				hp->pxUrlHandler[i].pfnEventHandler(MW_INIT, &hp->pxUrlHandler[i], hp)) {
				//remove the URL handler
				hp->pxUrlHandler[i].pfnUrlHandler = NULL;
			}
		}
	}

#ifdef _7Z
	hp->szctx = SzInit();
#endif

	if (!(hp->listenSocket = _mwStartListening(hp))) {
		DBG("Error listening on port %d\n", hp->httpPort);
		return -1;
	}

#ifndef WINCE
	hp->stats.startTime = time(NULL);
#else
	hp->stats.startTime = GetTickCount() >> 10;
#endif
	hp->bKillWebserver = FALSE;
	hp->bWebserverRunning = TRUE;
	if (!hp->tmSocketExpireTime) {
		hp->tmSocketExpireTime = HTTP_EXPIRATION_TIME;
	}
	return 0;
}
//------------------------------------------------------------------------------
int mwServerShutdown(HttpParam* hp)
{
	int i;
	if (hp->bKillWebserver || !hp->bWebserverRunning) {
		return -1;
	}

	DBG("Shutting down...\n");
	_mwCloseAllConnections(hp);

	// signal webserver thread to quit
	hp->bKillWebserver = TRUE;

	// and wait for thread to exit
	for (i = 0; hp->bWebserverRunning && i < 30; i++) {
		msleep(100);
	}

#ifdef _7Z
	SzUninit(hp->szctx);
#endif

	if (!hp->bWebserverRunning) {
		DBG("Webserver shutdown complete\n");
	}

	return 0;
}
//------------------------------------------------------------------------------
int mwGetLocalFileName(HttpFilePath* hfp)
{
	char ch;
	char *p = (char*)hfp->cFilePath;
	char *s = (char*)hfp->pchHttpPath;
	char *upLevel = NULL;

	hfp->pchExt = NULL;
	hfp->fTailSlash = 0;
	if (*s == '~') {
		s++;
	}
	else if (hfp->pchRootPath) {
		p += _mwStrCopy(hfp->cFilePath, hfp->pchRootPath);
		if (*(p - 1) != SLASH) {
			*p = SLASH;
			*(++p) = 0;
		}
	}
	while ((ch = *s) && ch != '?' && (int)(p - hfp->cFilePath)<sizeof(hfp->cFilePath) - 1) {
		if (ch == '%') {
			*(p++) = _mwDecodeCharacter(++s);
			s += 2;
		}
		else if (ch == '/') {
			*p = SLASH;
			upLevel = (++p);
			while (*(++s) == '/');
			continue;
		}
		else if (ch == '+') {
			*(p++) = ' ';
			s++;
		}
		else if (ch == '.') {
			if (upLevel && !memcmp(s + 1, "./", 2)) {
				s += 2;
				p = upLevel;
			}
			else {
				*(p++) = '.';
				hfp->pchExt = p;
				while (*(++s) == '.');	//avoid '..' appearing in filename for security issue
			}
		}
		else {
			*(p++) = *(s++);
		}
	}
	if (*(p - 1) == SLASH) {
		p--;
		hfp->fTailSlash = 1;
	}
	*p = 0;
	return (int)(p - hfp->cFilePath);
}
//------------------------------------------------------------------------------
void* mwHttpLoop(void* _hp)
{
	HttpParam *hp = (HttpParam*)_hp;
	HttpSocket *phsSocketCur;
	SOCKET socket;
	struct sockaddr_in sinaddr;
	int iRc;
	int i;

	// main processing loop
	while (!hp->bKillWebserver) {
		time_t tmCurrentTime;
		SOCKET iSelectMaxFds;
		fd_set fdsSelectRead;
		fd_set fdsSelectWrite;

		// clear descriptor sets
		FD_ZERO(&fdsSelectRead);
		FD_ZERO(&fdsSelectWrite);
		FD_SET(hp->listenSocket, &fdsSelectRead);
		iSelectMaxFds = hp->listenSocket;

		// get current time
#ifndef WINCE
		tmCurrentTime = time(NULL);
#else
		tmCurrentTime = GetTickCount() >> 10;
#endif
		// build descriptor sets and close timed out sockets
		for (i = 0; i < hp->maxClients; i++) {
			phsSocketCur = hp->hsSocketQueue + i;

			// get socket fd
			socket = phsSocketCur->socket;
			if (!socket) continue;

			{
				int iError = 0;
				int iOptSize = sizeof(int);
				if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&iError, &iOptSize)) {
					// if a socket contains a error, close it
					SYSLOG(LOG_INFO, "[%d] Socket no longer vaild.\n", socket);
					phsSocketCur->flags = FLAG_CONN_CLOSE;
					_mwCloseSocket(hp, phsSocketCur);
					continue;
				}
			}
			// check expiration timer (for non-listening, in-use sockets)
			if (tmCurrentTime > phsSocketCur->tmExpirationTime) {
				SYSLOG(LOG_INFO, "[%d] Http socket expired\n", phsSocketCur->socket);
				hp->stats.timeOutCount++;
				// close connection
				phsSocketCur->flags = FLAG_CONN_CLOSE;
				_mwCloseSocket(hp, phsSocketCur);
			}
			else {
				if (phsSocketCur->dwResumeTick) {
					// suspended
					if (phsSocketCur->dwResumeTick > GetTickCount())
						continue;
					else
						phsSocketCur->dwResumeTick = 0;
				}
				if (ISFLAGSET(phsSocketCur, FLAG_RECEIVING)) {
					// add to read descriptor set
					FD_SET(socket, &fdsSelectRead);
				}
				if (ISFLAGSET(phsSocketCur, FLAG_SENDING)) {
					// add to write descriptor set
					FD_SET(socket, &fdsSelectWrite);
				}
				// check if new max socket
				if (socket>iSelectMaxFds) {
					iSelectMaxFds = socket;
				}
			}
		}

		{
			struct timeval tvSelectWait;
			// initialize select delay
			tvSelectWait.tv_sec = 1;
			tvSelectWait.tv_usec = 0; // note: using timeval here -> usec not nsec

									  // and check sockets (may take a while!)
			iRc = select(iSelectMaxFds + 1, &fdsSelectRead, &fdsSelectWrite,
				NULL, &tvSelectWait);
		}
		if (iRc<0) {
			if (hp->bKillWebserver) break;
			DBG("Select error\n");
			msleep(1000);
			continue;
		}
		if (iRc>0) {
			// check which sockets are read/write able
			for (i = 0; i < hp->maxClients; i++) {
				BOOL bRead;
				BOOL bWrite;

				phsSocketCur = hp->hsSocketQueue + i;

				// get socket fd
				socket = phsSocketCur->socket;
				if (!socket) continue;

				// get read/write status for socket
				bRead = FD_ISSET(socket, &fdsSelectRead);
				bWrite = FD_ISSET(socket, &fdsSelectWrite);

				if ((bRead | bWrite) != 0) {
					//DBG("socket %d bWrite=%d, bRead=%d\n",phsSocketCur->socket,bWrite,bRead);
					// if readable or writeable then process
					if (bWrite && ISFLAGSET(phsSocketCur, FLAG_SENDING)) {
						iRc = _mwProcessWriteSocket(hp, phsSocketCur);
					}
					else if (bRead && ISFLAGSET(phsSocketCur, FLAG_RECEIVING)) {
						iRc = _mwProcessReadSocket(hp, phsSocketCur);
					}
					else {
						iRc = -1;
						DBG("Invalid socket state (flag: %08x)\n", phsSocketCur->flags);
					}
					if (!iRc) {
						// and reset expiration timer
#ifndef WINCE
						phsSocketCur->tmExpirationTime = time(NULL) + hp->tmSocketExpireTime;
#else
						phsSocketCur->tmExpirationTime = (GetTickCount() >> 10) + hp->tmSocketExpireTime;
#endif
					}
					else {
						SETFLAG(phsSocketCur, FLAG_CONN_CLOSE);
						_mwCloseSocket(hp, phsSocketCur);
					}
				}
			}

			// check if any socket to accept and accept the socket
			if (FD_ISSET(hp->listenSocket, &fdsSelectRead)) {
				// find empty slot
				phsSocketCur = 0;
				for (i = 0; i < hp->maxClients; i++) {
					if (hp->hsSocketQueue[i].socket == 0) {
						phsSocketCur = hp->hsSocketQueue + i;
						break;
					}
				}

				if (!phsSocketCur) {
					DBG("WARNING: clientCount:%d > maxClients:%d\n", hp->stats.clientCount, hp->maxClients);
					_mwDenySocket(hp, &sinaddr);
					continue;
				}

				phsSocketCur->socket = _mwAcceptSocket(hp, &sinaddr);
				if (phsSocketCur->socket == 0) continue;
				phsSocketCur->ipAddr.laddr = ntohl(sinaddr.sin_addr.s_addr);
				SYSLOG(LOG_INFO, "[%d] IP: %d.%d.%d.%d\n",
					phsSocketCur->socket,
					phsSocketCur->ipAddr.caddr[3],
					phsSocketCur->ipAddr.caddr[2],
					phsSocketCur->ipAddr.caddr[1],
					phsSocketCur->ipAddr.caddr[0]);

				hp->stats.clientCount++;

				//fill structure with data
				_mwInitSocketData(phsSocketCur);
				phsSocketCur->request.pucPayload = 0;
#ifndef WINCE
				phsSocketCur->tmAcceptTime = time(NULL);
#else
				phsSocketCur->tmAcceptTime = GetTickCount() >> 10;
#endif
				phsSocketCur->tmExpirationTime = phsSocketCur->tmAcceptTime + hp->tmSocketExpireTime;
				phsSocketCur->iRequestCount = 0;
				DBG("Connected clients: %d\n", hp->stats.clientCount);

				//update max client count
				if (hp->stats.clientCount>hp->stats.clientCountMax) hp->stats.clientCountMax = hp->stats.clientCount;
			}
		}
		else {
			//DBG("Select Timeout\n");
			// select timeout
			// call idle event
			if (hp->pfnIdleCallback) {
				(*hp->pfnIdleCallback)(hp);
			}
		}
	}

	// cleanup
	_mwCloseAllConnections(hp);
	SYSLOG(LOG_INFO, "Cleaning up...\n");
	for (i = 0; i < hp->maxClients; i++) {
		if (hp->hsSocketQueue[i].buffer) free(hp->hsSocketQueue[i].buffer);
	}
	for (i = 0; hp->pxUrlHandler[i].pchUrlPrefix; i++) {
		if (hp->pxUrlHandler[i].pfnUrlHandler && hp->pxUrlHandler[i].pfnEventHandler)
			hp->pxUrlHandler[i].pfnEventHandler(MW_UNINIT, &hp->pxUrlHandler[i], hp);
	}
	free(hp->hsSocketQueue);
	hp->hsSocketQueue = 0;

	// clear state vars
	hp->bKillWebserver = FALSE;
	hp->bWebserverRunning = FALSE;


	return NULL;
}//------------------------------------------------------------------------------
int mwParseQueryString(UrlHandlerParam* up)
{
	if (up->iVarCount == -1) {
		//parsing variables from query string
		unsigned char *p, *s;
		// get start of query string
		s = (unsigned char*)(strchr(up->pucRequest, '?'));
		if (s) {
			*(s++) = 0;
		}
		else if (ISFLAGSET(up->hs, FLAG_REQUEST_POST)) {
			s = (unsigned char*)(up->hs->request.pucPayload);
			if (s && s[0] == '<') s = 0;
		}
		if (s && *s) {
			int i;
			int n = 1;
			//get number of variables
			for (p = s; *p; p++) {
				if (*p < 32 || *p > 127)
					return 0;
				if (*p == '&') n++;
			}
			up->pxVars = (HttpVariables*)(calloc(n + 1, sizeof(HttpVariables)));
			up->iVarCount = n;
			//store variable name and value
			for (i = 0, p = s; i < n; p++) {
				switch (*p) {
				case '=':
					if (!(up->pxVars + i)->name) {
						*p = 0;
						(up->pxVars + i)->name = (char*)(s);
						s = p + 1;
					}
					break;
				case 0:
				case '&':
					*p = 0;
					if ((up->pxVars + i)->name) {
						(up->pxVars + i)->value = (char*)(s);
						mwDecodeString((char*)(s));
					}
					else {
						(up->pxVars + i)->name = (char*)(s);
						(up->pxVars + i)->value = (char*)(p);
					}
					s = p + 1;
					i++;
					break;
				}
			}
			(up->pxVars + n)->name = NULL;
		}
	}
	return up->iVarCount;
}
//------------------------------------------------------------------------------
void mwDecodeString(char* pchString)
{
	int bEnd = FALSE;
	char* pchInput = pchString;
	char* pchOutput = pchString;

	do {
		switch (*pchInput) {
		case '%':
			if (*(pchInput + 1) == '\0' || *(pchInput + 2) == '\0') {
				// something not right - terminate the string and abort
				*pchOutput = '\0';
				bEnd = TRUE;
			}
			else {
				*pchOutput = _mwDecodeCharacter(pchInput + 1);
				pchInput += 3;
			}
			break;
		case '+':
			*pchOutput = ' ';
			pchInput++;
			break;
		case '\0':
			bEnd = TRUE;
			// drop through
		default:
			// copy character
			*pchOutput = *pchInput;
			pchInput++;
		}
		pchOutput++;
	} while (!bEnd);
}
//------------------------------------------------------------------------------
int mwGetContentType(const char *pchExtname)
{
	DWORD dwExt = 0;
	// check type of file requested
	if (pchExtname[1] == '\0') {
		return HTTPFILETYPE_OCTET;
	}
	else if (pchExtname[2] == '\0') {
		memcpy(&dwExt, pchExtname, 2);
		switch (GETDWORD(pchExtname) & 0xffdfdf) {
		case FILEEXT_JS: return HTTPFILETYPE_JS;
		case FILEEXT_TS: return HTTPFILETYPE_TS;
		}
	}
	else if (pchExtname[3] == '\0' || pchExtname[3] == '?') {
		//identify 3-char file extensions
		memcpy(&dwExt, pchExtname, sizeof(dwExt));
		switch (dwExt & 0xffdfdfdf) {
		case FILEEXT_HTM:	return HTTPFILETYPE_HTML;
		case FILEEXT_XML:	return HTTPFILETYPE_XML;
		case FILEEXT_XSL:	return HTTPFILETYPE_XML;
		case FILEEXT_TEXT:	return HTTPFILETYPE_TEXT;
		case FILEEXT_XUL:	return HTTPFILETYPE_XUL;
		case FILEEXT_CSS:	return HTTPFILETYPE_CSS;
		case FILEEXT_PNG:	return HTTPFILETYPE_PNG;
		case FILEEXT_JPG:	return HTTPFILETYPE_JPEG;
		case FILEEXT_GIF:	return HTTPFILETYPE_GIF;
		case FILEEXT_SWF:	return HTTPFILETYPE_SWF;
		case FILEEXT_MPA:	return HTTPFILETYPE_MPA;
		case FILEEXT_MPG:	return HTTPFILETYPE_MPEG;
		case FILEEXT_AVI:	return HTTPFILETYPE_AVI;
		case FILEEXT_MP4:	return HTTPFILETYPE_MP4;
		case FILEEXT_MOV:	return HTTPFILETYPE_MOV;
		case FILEEXT_264:	return HTTPFILETYPE_264;
		case FILEEXT_FLV:	return HTTPFILETYPE_FLV;
		case FILEEXT_3GP:	return HTTPFILETYPE_3GP;
		case FILEEXT_ASF:	return HTTPFILETYPE_ASF;
		case FILEEXT_SDP:	return HTTPFILETYPE_SDP;
		}
	}
	else if (pchExtname[4] == '\0' || pchExtname[4] == '?') {
		memcpy(&dwExt, pchExtname, sizeof(dwExt));
		//logic-and with 0xdfdfdfdf gets the uppercase of 4 chars
		switch (dwExt & 0xdfdfdfdf) {
		case FILEEXT_HTML:	return HTTPFILETYPE_HTML;
		case FILEEXT_MPEG:	return HTTPFILETYPE_MPEG;
		case FILEEXT_M3U8:	return HTTPFILETYPE_M3U8;
		}
	}
	return HTTPFILETYPE_OCTET;
}
//------------------------------------------------------------------------------
PFNFILEUPLOADCALLBACK mwFileUploadRegister(HttpParam *httpParam, PFNFILEUPLOADCALLBACK pfnUploadCb)
{
	PFNFILEUPLOADCALLBACK pfnUploadPrevCb = httpParam->pfnFileUpload;

	// save new CB
	if (pfnUploadCb == NULL) 
		return NULL;

	httpParam->pfnFileUpload = pfnUploadCb;

	// return previous CB (so app can chain onto it)
	return pfnUploadPrevCb;
}
//------------------------------------------------------------------------------
PFNPOSTCALLBACK mwPostRegister(HttpParam *httpParam, PFNPOSTCALLBACK pfnPostCb)
{
	PFNPOSTCALLBACK pfnPostPrevCb = httpParam->pfnPost;

	// save new CB
	if (pfnPostCb == NULL) return NULL;
	httpParam->pfnPost = pfnPostCb;

	// return previous CB (so app can chain onto it)
	return pfnPostPrevCb;
}
//------------------------------------------------------------------------------