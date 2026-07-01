#ifndef HTTP_HPP
#define HTTP_HPP

/* [PREPROCESSOR] */

#include <errno.h>

#include <limits>
#include <vector>

#include "HTTP_ASCII.hpp"
#include "HTTP_Errors.hpp"
#include "HTTP_Headers.hpp"
#include "HTTP_Request.hpp"
#include "HTTP_Response.hpp"
#include "conf.hpp"
#include "network/DataTypes.hpp"

#define HTTP_NAME_SIZ 4
#define INT_MAX_SIZ 10
#define HEX_MAX_SIZ 16

#define DEC_BASE 10
#define HEX_BASE 16

/* [ENUMS */

enum e_parse_state {

    /* common states */
    S_PARS_START_CR,
    S_PARS_START_LF,
    S_PARS_METHOD,
    S_PARS_URI,
    S_PARS_HTTP_NAME,
    S_PARS_HTTP_MAJOR,
    S_PARS_HTTP_MINOR,
    S_PARS_HTTP_MINOR_LF,

    S_PARS_HEADER_KEY,
    S_PARS_HEADER_OWS_PRE,
    S_PARS_HEADER_VALUE,
    S_PARS_HEADER_OWS_POST,
    S_PARS_ENDOF_HEADER_LF,

    S_PARS_BLANK_LINE_CR,
    S_PARS_BLANK_LINE_LF,

    S_PARS_INSPECT_REQUEST,

    /* content-length */
    S_PARS_RECV_BODY,

    /* transfer-encoding */
    S_PARS_ENCD_DATA,
    S_PARS_ENCD_SIZE,
    S_PARS_ENCD_SIZE_LF,
    S_PARS_ENCD_ASSESS_BODY_SIZE,
    S_PARS_ENCD_BODY,
    S_PARS_ENCD_BODY_CR,
    S_PARS_ENCD_BODY_LF,

    S_PARS_ENCD_EXT_START,
    S_PARS_ENCD_EXT_KEY,
    S_PARS_ENCD_EXT_VAL,
    S_PARS_ENCD_EXT_LF,

    S_PARS_ENCD_TRAIL_START,
    S_PARS_ENCD_TRAIL_KEY,
    S_PARS_ENCD_TRAIL_VAL,
    S_PARS_ENCD_TRAIL_CR,
    S_PARS_ENCD_TRAIL_LF,

    S_PARS_ENCD_END_CR,
    S_PARS_ENCD_END_LF,

    /* exit states */
    S_PARS_HEAD_DONE,
    S_PARS_BODY_DONE,
    S_PARS_ERROR
};

#define _IS_READ_ENCD_BODY(test_state) \
    ((test_state) >= S_PARS_ENCD_DATA && (test_state) <= S_PARS_ENCD_END_LF)
#define _IS_READ_HEADERS(test_state) \
    ((test_state) >= S_PARS_START_CR && (test_state) <= S_PARS_BLANK_LINE_LF)

/* [TYPEDEFS] */

typedef enum e_parse_state parse_state_t;

typedef struct conf_http conf_http_t;

/* [IMPLEMENTATION] */

class HTTP_Parser {
public:
    /* --------------[class semantics]---------------------------- */

    typedef std::vector<char> buffer_t;

    /* --------------[constructors]------------------------------- */

    HTTP_Parser(const conf_http_t& http_conf);
    HTTP_Parser(const HTTP_Parser& other);
    ~HTTP_Parser();

    HTTP_Parser& operator=(const HTTP_Parser& other);

    /* --------------[getters, setters]---------------------------- */

    parse_state_t state() const;
    void set_state(parse_state_t state);
    bool has_buffered_data() const;
    struct buffer cbuffer();

    /* --------------[parser config]------------------------------- */

    size_t http_max_method_siz() const;
    void set_http_max_method_siz(size_t size);

    size_t http_max_uri_siz() const;
    void set_http_max_uri_siz(size_t size);

    size_t http_max_header_key_siz() const;
    void set_http_max_header_key_siz(size_t size);

    size_t http_max_header_val_siz() const;
    void set_http_max_header_val_siz(size_t size);

    size_t http_max_header_amt_siz() const;
    void set_http_max_header_amt_siz(size_t size);

    size_t http_max_body_siz() const;
    void set_http_max_body_siz(size_t size);

    /* --------------[clearing functions]---------------------------- */

    void clear();
    void reset();

    /* --------------[parsing functions]---------------------------- */

    size_t read_request(HTTP_Request& request);
    void read_body();
    void bufferize(const char* buffer, size_t size);
    void trim_buffer(size_t pos);

private:
    /* --------------[private variables]----------------------------- */

    parse_state_t _state;

    std::string _token;
    std::string _token_key;
    std::string _token_val;

    buffer_t _buffer;
    struct buffer _char_buffer;

    size_t _http_max_method_siz;
    size_t _http_max_uri_siz;
    size_t _http_max_header_key_siz;
    size_t _http_max_header_val_siz;
    size_t _http_max_header_amt_siz;
    size_t _http_max_body_siz;

    size_t _rcvd_bytes;
    size_t _rcvd_header_key_bytes;
    size_t _rcvd_header_val_bytes;
    size_t _expected_bytes;
    size_t _total_expected_bytes;

    bool _in_dquote;
    bool _escape_next_char;

    /* --------------[private member functions]--------------------- */

    inline parse_state_t _append_byte(char c);
    inline parse_state_t _commit_token(HTTP_Request& request);
    inline parse_state_t _inspect_request(HTTP_Request& request);

    /* --------------[default http length values]------------------- */

    static const size_t _DFLT_HTTP_MAX_METHOD_SIZ = 32;
    static const size_t _DFLT_HTTP_MAX_URI_SIZ = 8192;
    static const size_t _DFLT_HTTP_MAX_HEADER_KEY_SIZ = 128;
    static const size_t _DFLT_HTTP_MAX_HEADER_VAL_SIZ = 8192;
    static const size_t _DFLT_HTTP_MAX_HEADER_AMT_SIZ = 100;
    static const size_t _DFLT_HTTP_MAX_BODY_SIZ = 1048573;
};

#endif
