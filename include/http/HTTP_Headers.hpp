#ifndef HTTP_HEADERS
#define HTTP_HEADERS

typedef enum http_enum_e {

    /* lists */
    HTTP_HEAD_ACCEPT,
    HTTP_HEAD_ACCEPT_ENCOD,
    HTTP_HEAD_ACCEPT_LANG,
    HTTP_HEAD_ACCEPT_RNG,
    HTTP_HEAD_ALLOW,
    HTTP_HEAD_AUTH_INFO,
    HTTP_HEAD_CONNECTION,
    HTTP_HEAD_CONTENT_ENCODING,
    HTTP_HEAD_CONTENT_LANG,
    HTTP_HEAD_SET_COOKIE,
    HTTP_HEAD_IF_MATCH,
    HTTP_HEAD_IF_NONE_MATCH,
    HTTP_HEAD_PROXY_AUTH,
    HTTP_HEAD_PROXY_AUTH_INFO,
    HTTP_HEAD_TE,
    HTTP_HEAD_TRAILER,
    HTTP_HEAD_UPGRADE,
    HTTP_HEAD_VARY,
    HTTP_HEAD_VIA,
    HTTP_HEAD_WWW_AUTH,
    HTTP_HEAD_TRANS_ENCODING,

    /* singletons */
    HTTP_HEAD_AUTHOR,
    HTTP_HEAD_CONTENT_LENGTH,
    HTTP_HEAD_CONTENT_LOC,
    HTTP_HEAD_CONTENT_RNG,
    HTTP_HEAD_CONTENT_TYPE,
    HTTP_HEAD_COOKIE,
    HTTP_HEAD_DATE,
    HTTP_HEAD_ETAG,
    HTTP_HEAD_EXPECT,
    HTTP_HEAD_FROM,
    HTTP_HEAD_HOST,
    HTTP_HEAD_IF_MODIF_SINCE,
    HTTP_HEAD_IF_RNG,
    HTTP_HEAD_IF_UNMODIF_SINCE,
    HTTP_HEAD_LAST_MODIF,
    HTTP_HEAD_LOC,
    HTTP_HEAD_MAX_FORWARDS,
    HTTP_HEAD_PROXY_AUTHOR,
    HTTP_HEAD_RNG,
    HTTP_HEAD_REFERER,
    HTTP_HEAD_RETRY_AFTER,
    HTTP_HEAD_SRV,
    HTTP_HEAD_USR_AGENT,

} http_header_enum_t;

typedef enum header_type_e { HEAD_SINGLETON, HEAD_LIST } header_type_t;

typedef struct http_header_e {
    http_header_enum_t enum_val;
    const char* header;

    header_type_t type;
} http_header_t;

const http_header_t headers[] = {

    {HTTP_HEAD_ACCEPT, "accept", HEAD_LIST},
    {HTTP_HEAD_ACCEPT_ENCOD, "accept-encoding", HEAD_LIST},
    {HTTP_HEAD_ACCEPT_LANG, "accept-language", HEAD_LIST},
    {HTTP_HEAD_ACCEPT_RNG, "accept-ranges", HEAD_LIST},
    {HTTP_HEAD_ALLOW, "allow", HEAD_LIST},
    {HTTP_HEAD_AUTH_INFO, "authentication-info", HEAD_LIST},
    {HTTP_HEAD_AUTHOR, "authorization", HEAD_SINGLETON},
    {HTTP_HEAD_CONNECTION, "connection", HEAD_LIST},
    {HTTP_HEAD_CONTENT_ENCODING, "content-encoding", HEAD_LIST},
    {HTTP_HEAD_CONTENT_LANG, "content-language", HEAD_LIST},
    {HTTP_HEAD_CONTENT_LENGTH, "content-length", HEAD_SINGLETON},
    {HTTP_HEAD_CONTENT_LOC, "content-location", HEAD_SINGLETON},
    {HTTP_HEAD_CONTENT_RNG, "content-range", HEAD_SINGLETON},
    {HTTP_HEAD_CONTENT_TYPE, "content-type", HEAD_SINGLETON},
    {HTTP_HEAD_COOKIE, "cookie", HEAD_SINGLETON},
    {HTTP_HEAD_SET_COOKIE, "set-cookie", HEAD_LIST},
    {HTTP_HEAD_DATE, "date", HEAD_SINGLETON},
    {HTTP_HEAD_ETAG, "eTag", HEAD_SINGLETON},
    {HTTP_HEAD_EXPECT, "expect", HEAD_SINGLETON},
    {HTTP_HEAD_FROM, "from", HEAD_SINGLETON},
    {HTTP_HEAD_HOST, "host", HEAD_SINGLETON},
    {HTTP_HEAD_IF_MATCH, "if-match", HEAD_LIST},
    {HTTP_HEAD_IF_MODIF_SINCE, "if-modified-since", HEAD_SINGLETON},
    {HTTP_HEAD_IF_NONE_MATCH, "if-none-match", HEAD_LIST},
    {HTTP_HEAD_IF_RNG, "if-range", HEAD_SINGLETON},
    {HTTP_HEAD_IF_UNMODIF_SINCE, "if-unmodified-since", HEAD_SINGLETON},
    {HTTP_HEAD_LAST_MODIF, "last-modified", HEAD_SINGLETON},
    {HTTP_HEAD_LOC, "location", HEAD_SINGLETON},
    {HTTP_HEAD_MAX_FORWARDS, "max-forwards", HEAD_SINGLETON},
    {HTTP_HEAD_PROXY_AUTH, "proxy-authenticate", HEAD_LIST},
    {HTTP_HEAD_PROXY_AUTH_INFO, "proxy-authentication-info", HEAD_LIST},
    {HTTP_HEAD_PROXY_AUTHOR, "proxy-authorization", HEAD_SINGLETON},
    {HTTP_HEAD_RNG, "range", HEAD_SINGLETON},
    {HTTP_HEAD_REFERER, "referer", HEAD_SINGLETON},
    {HTTP_HEAD_RETRY_AFTER, "retry-after", HEAD_SINGLETON},
    {HTTP_HEAD_SRV, "rerver", HEAD_SINGLETON},
    {HTTP_HEAD_TE, "tE", HEAD_LIST},
    {HTTP_HEAD_TRAILER, "trailer", HEAD_LIST},
    {HTTP_HEAD_TRANS_ENCODING, "transfer-encoding", HEAD_LIST},
    {HTTP_HEAD_UPGRADE, "upgrade", HEAD_LIST},
    {HTTP_HEAD_USR_AGENT, "user-agent", HEAD_SINGLETON},
    {HTTP_HEAD_VARY, "vary", HEAD_LIST},
    {HTTP_HEAD_VIA, "via", HEAD_LIST},
    {HTTP_HEAD_WWW_AUTH, "www-authenticate", HEAD_LIST},
};

inline const http_header_t* http_header(http_header_enum_t header) {
    static const unsigned int list_size = sizeof(headers) / sizeof(headers[0]);

    for (unsigned int i = 0; i < list_size; ++i) {
        if (headers[i].enum_val == header) {
            return &headers[i];
        }
    }
    return 0;
}

inline const http_header_t* http_header(const char* header) {
    static const unsigned int list_size = sizeof(headers) / sizeof(headers[0]);

    for (unsigned int i = 0; i < list_size; ++i) {
        if (headers[i].header == header) {
            return &headers[i];
        }
    }
    return 0;
}

#endif
