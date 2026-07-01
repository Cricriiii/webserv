#ifndef HTTP_ERRORS_HPP
#define HTTP_ERRORS_HPP

typedef struct {
    int code;
    const char* reason;
} http_error_t;

typedef enum e_http_error_range {
    E_HTTP_RNG_START = 100,
    E_HTTP_RNG_INFO = E_HTTP_RNG_START,
    E_HTTP_RNG_SUCCESS = 200,
    E_HTTP_RNG_REDIR = 300,
    E_HTTP_RNG_CLIENT_ERR = 400,
    E_HTTP_RNG_SERVER_ERR = 500,
    E_HTTP_RNG_END = 511
} http_error_range_t;

#define HTTP_IS_INFO(errval) ((errval) / 100 == 1)
#define HTTP_IS_SUCCESS(errval) ((errval) / 100 == 2)
#define HTTP_IS_REDIR(errval) ((errval) / 100 == 3)
#define HTTP_IS_CLI_ERR(errval) ((errval) / 100 == 4)
#define HTTP_IS_SRV_ERR(errval) ((errval) / 100 == 5)
#define HTTP_IS_ERR(errval) (HTTP_IS_CLI_ERR(errval) || HTTP_IS_SRV_ERR(errval))

typedef enum e_http_status {
    /* informational */
    E_HTTP_CONTINUE = 100,
    E_HTTP_SWITCHING_PROTOCOLS = 101,
    E_HTTP_PROCESSING = 102,
    E_HTTP_EARLY_HINTS = 103,

    /* success */
    E_HTTP_OK = 200,
    E_HTTP_CREATED = 201,
    E_HTTP_ACCEPTED = 202,
    E_HTTP_NON_AUTHORIT_INFO = 203,
    E_HTTP_NO_CONTENT = 204,
    E_HTTP_RESET_CONTENT = 205,
    E_HTTP_PARTIAL_CONTENT = 206,
    E_HTTP_MULTI_STATUS = 207,
    E_HTTP_ALREADY_REPORTED = 208,
    E_HTTP_IM_USED = 226,

    /* redirection */
    E_HTTP_MULTIPLE_CHOICES = 300,
    E_HTTP_MOVED_PERMANENTLY = 301,
    E_HTTP_FOUND = 302,
    E_HTTP_SEE_OTHER = 303,
    E_HTTP_NOT_MODIFIED = 304,
    E_HTTP_TEMPORARY_REDIRECT = 307,
    E_HTTP_PERMANENT_REDIRECT = 308,

    /* client error */
    E_HTTP_BAD_REQUEST = 400,
    E_HTTP_UNAUTHORIZED = 401,
    E_HTTP_PAYMENT_REQUIRED = 402,
    E_HTTP_FORBIDDEN = 403,
    E_HTTP_NOT_FOUND = 404,
    E_HTTP_METHOD_NOT_ALLOWED = 405,
    E_HTTP_NOT_ACCEPTABLE = 406,
    E_HTTP_PROXY_AUTHENT_REQ = 407,
    E_HTTP_REQUEST_TIMEOUT = 408,
    E_HTTP_CONFLICT = 409,
    E_HTTP_GONE = 410,
    E_HTTP_LENGTH_REQUIRED = 411,
    E_HTTP_PRECONDITION_FAILED = 412,
    E_HTTP_CONTENT_TOO_LARGE = 413,
    E_HTTP_URI_TOO_LONG = 414,
    E_HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    E_HTTP_RANGE_NOT_SATISFIABLE = 416,
    E_HTTP_EXPECTATION_FAILED = 417,
    E_HTTP_IM_A_TEAPOT = 418,
    E_HTTP_MISDIRECTED_REQUEST = 421,
    E_HTTP_UNPROCESSABLE_CONTENT = 422,
    E_HTTP_LOCKED = 423,
    E_HTTP_FAILED_DEPENDENCY = 424,
    E_HTTP_TOO_EARLY = 425,
    E_HTTP_UPGRADE_REQUIRED = 426,
    E_HTTP_PRECONDITION_REQUIRED = 428,
    E_HTTP_TOO_MANY_REQUESTS = 429,
    E_HTTP_HEADER_FIELDS_TOO_LARGE = 431,
    E_HTTP_UNAVAILABLE_FOR_LEGAL_REASONS = 451,

    /* server error */
    E_HTTP_INTERNAL_SERVER_ERROR = 500,
    E_HTTP_NOT_IMPLEMENTED = 501,
    E_HTTP_BAD_GATEWAY = 502,
    E_HTTP_SERVICE_UNAVAILABLE = 503,
    E_HTTP_GATEWAY_TIMEOUT = 504,
    E_HTTP_VERSION_NOT_SUPPORTED = 505,
    E_HTTP_VARIANT_ALSO_NEGOTIATES = 506,
    E_HTTP_INSUFFICIENT_STORAGE = 507,
    E_HTTP_LOOP_DETECTED = 508,
    E_HTTP_NOT_EXTENDED = 510,
    E_HTTP_NET_AUTHENT_REQ = 511
} http_status_t;

static const http_error_t http_errors[] = {

    /* informational */
    {E_HTTP_CONTINUE, "Continue"},
    {E_HTTP_SWITCHING_PROTOCOLS, "Switching Protocols"},
    {E_HTTP_PROCESSING, "Processing"},
    {E_HTTP_EARLY_HINTS, "Early Hints"},

    /* success */
    {E_HTTP_OK, "OK"},
    {E_HTTP_CREATED, "Created"},
    {E_HTTP_ACCEPTED, "Accepted"},
    {E_HTTP_NON_AUTHORIT_INFO, "Non-Authoritative Information"},
    {E_HTTP_NO_CONTENT, "No Content"},
    {E_HTTP_RESET_CONTENT, "Reset Content"},
    {E_HTTP_PARTIAL_CONTENT, "Partial Content"},
    {E_HTTP_MULTI_STATUS, "Multi-Status"},
    {E_HTTP_ALREADY_REPORTED, "Already Reported"},
    {E_HTTP_IM_USED, "IM Used"},

    /* redirection */
    {E_HTTP_MULTIPLE_CHOICES, "Multiple Choices"},
    {E_HTTP_MOVED_PERMANENTLY, "Moved Permanently"},
    {E_HTTP_FOUND, "Found"},
    {E_HTTP_SEE_OTHER, "See Other"},
    {E_HTTP_NOT_MODIFIED, "Not Modified"},
    {E_HTTP_TEMPORARY_REDIRECT, "Temporary Redirect"},
    {E_HTTP_PERMANENT_REDIRECT, "Permanent Redirect"},

    /* client error */
    {E_HTTP_BAD_REQUEST, "Bad Request"},
    {E_HTTP_UNAUTHORIZED, "Unauthorized"},
    {E_HTTP_PAYMENT_REQUIRED, "Payment Required"},
    {E_HTTP_FORBIDDEN, "Forbidden"},
    {E_HTTP_NOT_FOUND, "Not Found"},
    {E_HTTP_METHOD_NOT_ALLOWED, "Method Not Allowed"},
    {E_HTTP_NOT_ACCEPTABLE, "Not Acceptable"},
    {E_HTTP_PROXY_AUTHENT_REQ, "Proxy Authentication Required"},
    {E_HTTP_REQUEST_TIMEOUT, "Request Timeout"},
    {E_HTTP_CONFLICT, "Conflict"},
    {E_HTTP_GONE, "Gone"},
    {E_HTTP_LENGTH_REQUIRED, "Length Required"},
    {E_HTTP_PRECONDITION_FAILED, "Precondition Failed"},
    {E_HTTP_CONTENT_TOO_LARGE, "Content Too Large"},
    {E_HTTP_URI_TOO_LONG, "URI Too Long"},
    {E_HTTP_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
    {E_HTTP_RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
    {E_HTTP_EXPECTATION_FAILED, "Expectation Failed"},
    {E_HTTP_IM_A_TEAPOT, "I'm a Teapot"},
    {E_HTTP_MISDIRECTED_REQUEST, "Misdirected Request"},
    {E_HTTP_UNPROCESSABLE_CONTENT, "Unprocessable Content"},
    {E_HTTP_LOCKED, "Locked"},
    {E_HTTP_FAILED_DEPENDENCY, "Failed Dependency"},
    {E_HTTP_TOO_EARLY, "Too Early"},
    {E_HTTP_UPGRADE_REQUIRED, "Upgrade Required"},
    {E_HTTP_PRECONDITION_REQUIRED, "Precondition Required"},
    {E_HTTP_TOO_MANY_REQUESTS, "Too Many Requests"},
    {E_HTTP_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
    {E_HTTP_UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},

    /* server error */
    {E_HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error"},
    {E_HTTP_NOT_IMPLEMENTED, "Not Implemented"},
    {E_HTTP_BAD_GATEWAY, "Bad Gateway"},
    {E_HTTP_SERVICE_UNAVAILABLE, "Service Unavailable"},
    {E_HTTP_GATEWAY_TIMEOUT, "Gateway Timeout"},
    {E_HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
    {E_HTTP_VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates"},
    {E_HTTP_INSUFFICIENT_STORAGE, "Insufficient Storage"},
    {E_HTTP_LOOP_DETECTED, "Loop Detected"},
    {E_HTTP_NOT_EXTENDED, "Not Extended"},
    {E_HTTP_NET_AUTHENT_REQ, "Network Authentication Required"}};

__attribute__((always_inline)) inline const char* http_error_reason(
    int error_code) {
    static int err_list_size = sizeof(http_errors) / sizeof(*http_errors);
    for (int i = 0; i < err_list_size; ++i) {
        if (http_errors[i].code == error_code) {
            return http_errors[i].reason;
        }
    }
    return "Unknown error";
}

#endif
