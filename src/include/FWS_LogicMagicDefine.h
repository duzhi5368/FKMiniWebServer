#ifndef FWS_LogicMagicDefine
#define FWS_LogicMagicDefine

#define MAXPOSTPARAMS 50
#define MAXPOSTREDIRECTFILENAME (200)

#define FLAG_REQUEST_GET		0x1
#define FLAG_REQUEST_POST		0x2
#define FLAG_REQUEST_OPTIONS	0x4
#define FLAG_REQUEST_DESCRIBE	0x8
#define FLAG_REQUEST_SETUP		0x10
#define FLAG_REQUEST_PLAY		0x20
#define FLAG_REQUEST_TEARDOWN	0x40
#define FLAG_HEADER_SENT		0x80
#define FLAG_CONN_CLOSE			0x100
#define FLAG_SUBST				0x200
#define FLAG_AUTHENTICATION		0x400
#define FLAG_MORE_CONTENT		0x800
#define FLAG_TO_FREE			0x1000
#define FLAG_CHUNK				0x2000
#define FLAG_CLOSE_CALLBACK     0x4000
#define FLAG_DATA_FILE			0x10000
#define FLAG_DATA_RAW			0x20000
#define FLAG_DATA_FD			0x40000
#define FLAG_DATA_REDIRECT		0x80000
#define FLAG_DATA_STREAM		0x100000
#define FLAG_CUSTOM_HEADER		0x200000
#define FLAG_MULTIPART			0x400000
#define FLAG_SENDING			0x40000000
#define FLAG_RECEIVING			0x80000000

#define HTTP_BUFFER_SIZE (128*1024 /*bytes*/)
#define HTTP_MAX_CLIENTS 255
#define HTTP_MAX_CLIENTS_PER_IP 16

#define AUTH_NO_NEED (0)
#define AUTH_SUCCESSED (1)
#define AUTH_REQUIRED (2)
#define AUTH_FAILED (-1)

#define MAX_AUTH_INFO_LEN 64

#define FLAG_DIR_LISTING 1
#define FLAG_DISABLE_RANGE 2

///////////////////////////////////////////////////////////////////////
// Return codes
///////////////////////////////////////////////////////////////////////
// for post callback
#define WEBPOST_OK                (0)
#define WEBPOST_AUTHENTICATED     (1)
#define WEBPOST_NOTAUTHENTICATED  (2)
#define WEBPOST_AUTHENTICATIONON  (3)
#define WEBPOST_AUTHENTICATIONOFF (4)

// for multipart file uploads
#define HTTPUPLOAD_MORECHUNKS     (0)
#define HTTPUPLOAD_FIRSTCHUNK     (1)
#define HTTPUPLOAD_LASTCHUNK      (2)

#endif // FWS_LogicMagicDefine