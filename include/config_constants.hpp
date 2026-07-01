#ifndef CONFIG_CONTANTS_HPP
#define CONFIG_CONTANTS_HPP

/*	Application		*/
#define APP_NAME "webserv/1.0"

#define ENABLE_IO_HEXDUMP 0

#define HTTP_ONLY 1

#define HTTP_VER_MAJOR 1
#define HTTP_VER_MINOR 1

/*	SessionManager	*/

#define SESSION_ID_SIZ 8
static char session_id_charset[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#define SESSION_TIMEOUT 1800

/*	CGI Timeout		*/

#define CGI_TIMEOUT 2000000

#endif
