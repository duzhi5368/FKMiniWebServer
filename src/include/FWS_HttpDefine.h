#ifndef FWS_HttpDefine
#define FWS_HttpDefine

#ifndef HTTP_SERVER_NAME
#define HTTP_SERVER_NAME "FKMiniWebServer"
#endif
#define HTTP200_HEADER "%s %d %s\r\nServer: %s\r\nCache-control: no-cache\r\nPragma: no-cache\r\nConnection: %s\r\n"
#define HTTP200_HDR_EST_SIZE ((sizeof(HTTP200_HEADER)+256)&(-4))
#define HTTP403_HEADER "HTTP/1.1 403 Forbidden\r\nServer: %s\r\nContent-length: %d\r\nContent-Type: text/html\r\n\r\n"
#define HTTP404_HEADER "HTTP/1.1 404 Not Found\r\nServer: %s\r\nContent-length: %d\r\nContent-Type: text/html\r\n\r\n"
#define HTTP403_BODY "<html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1></body></html>"
#define HTTP404_BODY "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL has no content.</p></body></html>"
#define HTTPBODY_REDIRECT "<html><head><meta http-equiv=\"refresh\" content=\"0; URL=%s\"></head><body></body></html>"
#define HTTP301_HEADER "HTTP/1.1 301 Moved Permanently\r\nServer: %s\r\nLocation: %s\r\n\r\n"
#define HTTP401_HEADER "HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm=\"%s\"\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n"
#define HTTP_CONTENTLENGTH "Content-Length:"
#define HTTP_MULTIPARTHEADER "multipart/form-data"
#define HTTP_MULTIPARTCONTENT "Content-Disposition: form-data; name="
#define HTTP_MULTIPARTBOUNDARY "boundary="
#define HTTP_FILENAME "filename="
#define HTTP_HEADER_END "\r\n\r\n"
#define HTTP_SUBST_PATTERN (WORD)(('$' << 8) + '$')


#define FILEEXT_HTM DEFDWORD('H','T','M',0)
#define FILEEXT_XML DEFDWORD('X','M','L',0)
#define FILEEXT_XSL DEFDWORD('X','S','L',0)
#define FILEEXT_TEXT DEFDWORD('T','X','T',0)
#define FILEEXT_XUL DEFDWORD('X','U','L',0)
#define FILEEXT_GIF DEFDWORD('G','I','F',0)
#define FILEEXT_JPG DEFDWORD('J','P','G',0)
#define FILEEXT_PNG DEFDWORD('P','N','G',0)
#define FILEEXT_CSS DEFDWORD('C','S','S',0)
#define FILEEXT_JS DEFDWORD('J','S',0,0)
#define FILEEXT_SWF DEFDWORD('S','W','F',0)
#define FILEEXT_HTML DEFDWORD('H','T','M','L')
#define FILEEXT_MPG DEFDWORD('M','P','G',0)
#define FILEEXT_MPEG DEFDWORD('M','P','E','G')
#define FILEEXT_MPA DEFDWORD('M','P','3' - 32,0)
#define FILEEXT_AVI DEFDWORD('A','V','I',0)
#define FILEEXT_MP4 DEFDWORD('M','P','4' - 32,0)
#define FILEEXT_MOV DEFDWORD('M','O','V',0)
#define FILEEXT_FLV DEFDWORD('F','L','V',0)
#define FILEEXT_3GP DEFDWORD('3' - 32, 'G','P',0)
#define FILEEXT_ASF DEFDWORD('A','S','F',0)
#define FILEEXT_264 DEFDWORD('2' - 32, '6' - 32, '4' - 32, 0)
#define FILEEXT_TS DEFDWORD('T', 'S', 0, 0)
#define FILEEXT_M3U8 DEFDWORD('M', '3' - 32, 'U', '8' - 32)
#define FILEEXT_SDP DEFDWORD('S', 'D', 'P', 0)


#define HTTP_EXPIRATION_TIME (120/*secs*/)
#define HTTP_KEEPALIVE_TIME (15/*secs*/)
#define MAX_RECV_RETRIES (3/*times*/)
#define HTTPAUTHTIMEOUT   (300/*secs*/)
#define HTTPSUBSTEXPANSION (0/*bytes*/)
#define HTTPHEADERSIZE (512/*bytes*/)
#define HTTPSMALLBUFFER (256/*bytes*/)
#define HTTPMAXRECVBUFFER HTTP_BUFFER_SIZE
#define HTTPUPLOAD_CHUNKSIZE (HTTPMAXRECVBUFFER / 2/*bytes*/)
#define MAX_REQUEST_PATH_LEN (512/*bytes*/)
#define MAX_REQUEST_SIZE (2*1024 /*bytes*/)
#define MAX_POST_PAYLOAD_SIZE (256*1024 /*bytes*/)

#endif