#include <stdio.h>
#include "../include/FWS_UrlHandler.h"
#include "../include/FWS_MiniWebServerAPI.h"
#include "../include/FWS_PlatformIndependentLayer.h"
//------------------------------------------------------------------------------
int Func_UrlHandlerStats(UrlHandlerParam* param) {
	char *p;
	char buf[384];
	HttpStats *stats = &((HttpParam*)param->hp)->stats;
	HttpRequest *req = &param->hs->request;
	IPADDR ip = param->hs->ipAddr;
	HTTP_XML_NODE node;
	int bufsize = param->dataBytes;
	int ret = FLAG_DATA_RAW;

	mwGetHttpDateTime(time(NULL), buf, sizeof(buf));

	if (stats->clientCount>4) {
		param->pucBuffer = (char*)malloc(stats->clientCount * 256 + 1024);
		ret = FLAG_DATA_RAW | FLAG_TO_FREE;
	}

	p = param->pucBuffer;

	//generate XML
	WriteXmlHeader(&p, &bufsize, 10, 0, 0);

	WriteXmlString(&p, &bufsize, 0, "<ServerStats>");

	sprintf(buf, "%d.%d.%d.%d", ip.caddr[3], ip.caddr[2], ip.caddr[1], ip.caddr[0]);

	node.indent = 1;
	node.fmt = "%s";
	node.name = "ClientIP";
	node.value = buf;
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%d";
	node.name = "UpTime";
	node.value = (void*)(time(NULL) - stats->startTime);
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%d";
	node.name = "Clients";
	node.value = (void*)(stats->clientCount);
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%d";
	node.name = "ExpireTimeout";
	node.value = (void*)(stats->timeOutCount);
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%d";
	node.name = "MaxClients";
	node.value = (void*)(stats->clientCountMax);
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%u";
	node.name = "Requests";
	node.value = (void*)(stats->reqCount);
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%u";
	node.name = "FileSent";
	node.value = (void*)(stats->fileSentCount);
	WriteXmlLine(&p, &bufsize, &node, 0);

	node.fmt = "%u";
	node.name = "FileSentMB";
	node.value = (void*)(stats->fileSentBytes >> 20);
	WriteXmlLine(&p, &bufsize, &node, 0);

	WriteXmlString(&p, &bufsize, 1, "<Clients>");

	{
		HttpSocket *phsSocketCur;
		time_t curtime = time(NULL);
		int i;
		for (i = 0; i < ((HttpParam*)param->hp)->maxClients; i++) {
			phsSocketCur = ((HttpParam*)param->hp)->hsSocketQueue + i;
			if (!phsSocketCur->socket) continue;
			ip = phsSocketCur->ipAddr;
			sprintf(buf, "<Client ip=\"%d.%d.%d.%d\" requests=\"%d\" expire=\"%d\" speed=\"%u\" path=\"%s\"/>",
				ip.caddr[3], ip.caddr[2], ip.caddr[1], ip.caddr[0], phsSocketCur->iRequestCount, (int)(phsSocketCur->tmExpirationTime - curtime),
				(unsigned int)(phsSocketCur->response.sentBytes / (((curtime - phsSocketCur->tmAcceptTime) << 10) + 1)), phsSocketCur->request.pucPath);
			WriteXmlString(&p, &bufsize, 2, buf);
		}
	}

	WriteXmlString(&p, &bufsize, 1, "</Clients>");
	WriteXmlString(&p, &bufsize, 0, "</ServerStats>");

	//return data to server
	param->dataBytes = (int)p - (int)(param->pucBuffer);
	param->fileType = HTTPFILETYPE_XML;
	return ret;
}
//------------------------------------------------------------------------------
void* WriteContent(void* hdata)
{
	HANDLER_DATA* hData = (HANDLER_DATA*)hdata;
	char *p = hData->result;
	int i;
	for (i = 0; i < 10; i++, p++) {
		*p = '0' + i;
		msleep(100);
	}
	*p = 0;
	return 0;
}

int Func_UrlHandlerAsyncData(UrlHandlerParam* param) {
	int ret = FLAG_DATA_STREAM | FLAG_TO_FREE;
	HANDLER_DATA* hdata = (HANDLER_DATA*)param->hs->ptr;

	if (param->pucBuffer) {
		if (!hdata) {
			// first invoke
			param->hs->ptr = calloc(1, sizeof(HANDLER_DATA));
			hdata = (HANDLER_DATA*)(param->hs->ptr);
			ThreadCreate(&hdata->thread, WriteContent, hdata);
			param->dataBytes = 0;
		}
		else {
			if (hdata->state == 1) {
				// done
				ret = 0;
			}
			else if (ThreadWait(hdata->thread, 10, 0)) {
				// data not ready
				param->dataBytes = 0;
			}
			else {
				// data ready
				strcpy(param->pucBuffer, hdata->result);
				param->dataBytes = strlen(param->pucBuffer);
				hdata->state = 1;
			}
		}
	}
	else {
		// cleanup
		ret = 0;
	}
	param->fileType = HTTPFILETYPE_TEXT;
	return ret;
}
//------------------------------------------------------------------------------
#ifdef _7Z
int Func_UrlHandler7Zip(UrlHandlerParam* param) {
	HttpRequest *req = &param->hs->request;
	HttpParam *hp = (HttpParam*)param->hp;
	char *path;
	char *filename;
	void *content;
	int len;
	char *p = strchr(req->pucPath, '/');
	if (p) p = strchr(p + 1, '/');
	if (!p) return 0;
	filename = p + 1;
	*p = 0;
	path = (char*)malloc(strlen(req->pucPath) + strlen(hp->pchWebPath) + 5);
	sprintf(path, "%s/%s.7z", hp->pchWebPath, req->pucPath);
	*p = '/';

	if (!IsFileExist(path)) {
		free(path);
		return 0;
	}

	len = SzExtractContent(hp->szctx, path, filename, &content);
	free(path);
	if (len < 0) return 0;

	p = strrchr(filename, '.');
	param->fileType = p ? mwGetContentType(p + 1) : HTTPFILETYPE_OCTET;
	param->dataBytes = len;
	param->pucBuffer = content;
	return FLAG_DATA_RAW;
}
#endif
//------------------------------------------------------------------------------
int DefaultWebFileUploadCallback(HttpMultipart *pxMP, OCTET *poData, size_t dwDataChunkSize, char* pchWebPath)
{
	// Do nothing with the data
	int fd = (int)pxMP->pxCallBackData;
	if (!poData) {
		// to cleanup
		if (fd > 0) {
			close(fd);
			pxMP->pxCallBackData = NULL;
		}
		return 0;
	}
	if (!fd) {
		char filename[256];
		snprintf(filename, sizeof(filename), "%s/%s", pchWebPath, pxMP->pchFilename);
		fd = open(filename, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0);
		pxMP->pxCallBackData = (void*)fd;
	}
	if (fd <= 0) return -1;
	write(fd, poData, dwDataChunkSize);
	if (pxMP->oFileuploadStatus & HTTPUPLOAD_LASTCHUNK) {
		close(fd);
		pxMP->pxCallBackData = NULL;
	}
	printf("Received %u bytes for multipart upload file %s\n", dwDataChunkSize, pxMP->pchFilename);
	return 0;
}
//------------------------------------------------------------------------------
int DefaultWebSubstCallback(SubstParam* sp)
{
	// the maximum length of variable value should never exceed the number
	// given by sp->iMaxValueBytes
	if (!strcmp(sp->pchParamName, "mykeyword")) {
		return sprintf(sp->pchParamValue, "%d", 1234);
	}
	return -1;
}
//------------------------------------------------------------------------------
int DefaultWebPostCallback(PostParam* pp)
{
	int iReturn = WEBPOST_OK;

	// by default redirect to config page
	//strcpy(pp->chFilename,"index.htm");

	return iReturn;
}
//------------------------------------------------------------------------------