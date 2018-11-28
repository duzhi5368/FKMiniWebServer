//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "../include/FWS_MiniWebServerAPI.h"
#include "../include/FWS_PlatformIndependentLayer.h"
#include "../include/FWS_UrlHandler.h"
#include "../include/FWS_Version.h"
//------------------------------------------------------------------------------
#ifdef WIN32
#pragma comment(lib, "WSock32.Lib")
#endif
//------------------------------------------------------------------------------
HttpParam httpParam;
//------------------------------------------------------------------------------
UrlHandler DefaultUrlHandlerList[] = {
	{ "stats", Func_UrlHandlerStats, NULL },
	{ "async", Func_UrlHandlerAsyncData, NULL },
#ifdef _7Z
	{ "7z", Func_UrlHandler7Zip, NULL },
#endif
	{ NULL },
};
//------------------------------------------------------------------------------
void AppShutdown()
{
	//shutdown server
	mwServerShutdown(&httpParam);
	fclose(fpLog);
	UninitSocket();
}
//------------------------------------------------------------------------------
int AppQuit(int arg) {
	static int quitting = 0;
	if (quitting) return 0;
	quitting = 1;
	if (arg) {
		printf("\nCaught signal (%d). MiniWeb shutting down...\n", arg);
	}
	AppShutdown();
	return 0;
}
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

	fprintf(stderr, "FKMiniWebServer Ver %d.%d (built on %s)\n",
		VER_MAJOR, VER_MINOR, __DATE__);
	fprintf(stderr, "---------------------------------------\n");

	// registe quit sign
	// 注册退出处理
#ifdef WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)AppQuit, TRUE);
#else
	signal(SIGINT, (void *)AppQuit);
	signal(SIGTERM, (void *)AppQuit);
	signal(SIGPIPE, SIG_IGN);
#endif

	// init configs
	// 初始化配置
	mwInitParam(&httpParam);
	GetFullPath(httpParam.pchWebPath, argv[0], "htdocs");
	httpParam.pxUrlHandler = DefaultUrlHandlerList;

	// parsing command line arguments
	// 命令行参数解析
	{
		int i;
		for (i = 1; i<argc; i++) {
			if (argv[i][0] == '-') {
				switch (argv[i][1]) {
				case 'h':
					fprintf(stderr, "Usage: FKMiniWebServer	-h	: display this help screen\n"
						"		-v	: log status/error info\n"
						"		-p	: specifiy http port [default 80]\n"
						"		-r	: specify http document directory [default htdocs]\n"
						"		-l	: specify log file\n"
						"		-m	: specifiy max clients [default 32]\n"
						"		-M	: specifiy max clients per IP\n"
						"		-s	: specifiy download speed limit in KB/s [default: none]\n"
						"		-n	: disallow multi-part download [default: allow]\n"
						"		-d	: disallow directory listing [default ON]\n\n");
					fflush(stderr);
					exit(1);

				case 'p':
					if ((++i) < argc) {
						httpParam.httpPort = atoi(argv[i]);
					}
					break;
				case 'r':
					if ((++i) < argc) {
						strncpy(httpParam.pchWebPath, argv[i], sizeof(httpParam.pchWebPath) - 1);
					}
					break;
				case 'l':
					if ((++i) < argc) {
						fpLog = freopen(argv[i], "w", stderr);
					}
					break;
				case 'm':
					if ((++i) < argc) { 
						httpParam.maxClients = atoi(argv[i]); 
					}
					break;
				case 'M':
					if ((++i) < argc) {
						httpParam.maxClientsPerIP = atoi(argv[i]);
					}
					break;
				case 's':
					if ((++i) < argc) {
						httpParam.maxDownloadSpeed = atoi(argv[i]);
					}
					break;
				case 'n':
					httpParam.flags |= FLAG_DISABLE_RANGE;
					break;
				case 'd':
					httpParam.flags &= ~FLAG_DIR_LISTING;
					break;
				}
			}
		}
	}

	// parsing url event handlers
	// 解析Url时间处理器
	{
		int i;
		int error = 0;
		for (i = 0; DefaultUrlHandlerList[i].pchUrlPrefix; i++) {
			if (DefaultUrlHandlerList[i].pfnEventHandler) {
				if (DefaultUrlHandlerList[i].pfnEventHandler(MW_PARSE_ARGS, DefaultUrlHandlerList[i].pfnEventHandler, &httpParam)) {
					error++;
				}
			}
		}
		if (error > 0) {
			printf("Error parsing command line options\n");
			return -1;
		}
	}

	// init
	InitSocket();

	// print infos
	// 输出一些信息
	{
		printf("Host: %s:%d\n", GetLocalAddrString(), httpParam.httpPort);
		printf("Web root: %s\n", httpParam.pchWebPath);
		printf("Max clients: %d, Max clients per IP: (%d)\n", httpParam.maxClients, httpParam.maxClientsPerIP);
		int n;
		for (n = 0; DefaultUrlHandlerList[n].pchUrlPrefix; n++) {};
		printf("URL handlers: %d\n", n);
		if (httpParam.flags & FLAG_DIR_LISTING) {
			printf("Dir listing: Enabled\n");
		}
		if (httpParam.flags & ~FLAG_DISABLE_RANGE) {
			printf("Byte-range: Enabled\n");
		}
		fprintf(stderr, "---------------------------------------\n");
	}

	// start server
	if (mwServerStart(&httpParam)) {
		printf("Error starting HTTP server\n");
	}
	else {
		mwHttpLoop(&httpParam);
	}

	// shutdown
	AppShutdown();

	return 0;
}
//------------------------------------------------------------------------------