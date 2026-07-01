#include "http/HTTP_Parser.hpp"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <limits>

#include "tools/char_tools.hpp"
#include "tools/define_size.hpp"

/* [FSM
 * STATES] */

#define REDISPATCH goto redispatch
#define IS_STATE(TEST_STATE) (_state == (TEST_STATE))
#define IS_ERR(TEST_STATE) ((TEST_STATE) == S_PARS_ERROR)
#define SET_STATE(NEW_STATE) (_state = (NEW_STATE))
#define SET_ERROR (_state = S_PARS_ERROR)
#define GOTO(NEW_STATE)   \
    SET_STATE(NEW_STATE); \
    REDISPATCH
#define GOTO_ERR \
    SET_ERROR;   \
    REDISPATCH
#define REJECT_REQUEST(STATE) \
    request.status = (STATE); \
    GOTO_ERR
#define IFN_ERR_SET_STATE(NEW_STATE) \
    if (IS_STATE(S_PARS_ERROR)) {    \
        REDISPATCH;                  \
    } else {                         \
        SET_STATE(NEW_STATE);        \
    }
#define IFN_ERR_GOTO(NEW_STATE)   \
    if (IS_STATE(S_PARS_ERROR)) { \
        REDISPATCH;               \
    } else {                      \
        GOTO(NEW_STATE);          \
    }

/* [CONSTRUCTORS,
 * DESTRUCTORS] */

HTTP_Parser::HTTP_Parser(const conf_http_t& http_conf)
    : _state(S_PARS_START_CR),
      _token(),
      _token_key(),
      _token_val(),
      _buffer(),
      _char_buffer(),
      _http_max_method_siz(0),
      _http_max_uri_siz(0),
      _http_max_header_key_siz(0),
      _http_max_header_val_siz(0),
      _http_max_header_amt_siz(0),
      _http_max_body_siz(0),
      _rcvd_bytes(0),
      _rcvd_header_key_bytes(0),
      _rcvd_header_val_bytes(0),
      _expected_bytes(0),
      _total_expected_bytes(0),
      _in_dquote(false),
      _escape_next_char(false) {
    _http_max_method_siz =
        define_size(_DFLT_HTTP_MAX_METHOD_SIZ, http_conf.http_max_method_siz);
    _http_max_uri_siz =
        define_size(_DFLT_HTTP_MAX_URI_SIZ, http_conf.http_max_uri_siz);
    _http_max_header_key_siz = define_size(_DFLT_HTTP_MAX_HEADER_KEY_SIZ,
                                           http_conf.http_max_header_key_siz);
    _http_max_header_val_siz = define_size(_DFLT_HTTP_MAX_HEADER_VAL_SIZ,
                                           http_conf.http_max_header_val_siz);
    _http_max_header_amt_siz = define_size(_DFLT_HTTP_MAX_HEADER_VAL_SIZ,
                                           http_conf.http_max_header_amt_siz);
    _http_max_body_siz =
        define_size(_DFLT_HTTP_MAX_BODY_SIZ, http_conf.http_max_body_siz);
}

HTTP_Parser::HTTP_Parser(const HTTP_Parser& other)
    : _state(other._state),
      _token(other._token),
      _token_key(other._token_key),
      _token_val(other._token_val),
      _buffer(other._buffer),
      _char_buffer(other._char_buffer),
      _http_max_method_siz(other._http_max_method_siz),
      _http_max_uri_siz(other._http_max_uri_siz),
      _http_max_header_key_siz(other._http_max_header_key_siz),
      _http_max_header_val_siz(other._http_max_header_val_siz),
      _http_max_header_amt_siz(other._http_max_header_amt_siz),
      _http_max_body_siz(other._http_max_body_siz),
      _rcvd_bytes(other._rcvd_bytes),
      _rcvd_header_key_bytes(other._rcvd_header_key_bytes),
      _rcvd_header_val_bytes(other._rcvd_header_val_bytes),
      _expected_bytes(other._expected_bytes),
      _total_expected_bytes(other._total_expected_bytes),
      _in_dquote(other._in_dquote),
      _escape_next_char(other._escape_next_char) {
}

HTTP_Parser::~HTTP_Parser() {
}

HTTP_Parser& HTTP_Parser::operator=(const HTTP_Parser& other) {
    if (this != &other) {
        _state = other._state;
        _token = other._token;
        _token_key = other._token_key;
        _token_val = other._token_val;
        _buffer = other._buffer;
        _char_buffer = other._char_buffer;
        _http_max_method_siz = other._http_max_method_siz;
        _http_max_uri_siz = other._http_max_uri_siz;
        _http_max_header_key_siz = other._http_max_header_key_siz,
        _http_max_header_val_siz = other._http_max_header_val_siz,
        _http_max_header_amt_siz = other._http_max_header_amt_siz,
        _http_max_body_siz = other._http_max_body_siz;
        _rcvd_bytes = other._rcvd_bytes;
        _rcvd_header_key_bytes = other._rcvd_header_key_bytes;
        _rcvd_header_val_bytes = other._rcvd_header_val_bytes;
        _expected_bytes = other._expected_bytes;
        _total_expected_bytes = other._total_expected_bytes;
        _in_dquote = other._in_dquote;
        _escape_next_char = other._escape_next_char;
    }
    return *this;
}

/* [GETTERS,
 * SETTERS] */

parse_state_t HTTP_Parser::state() const {
    return _state;
}

void HTTP_Parser::set_state(parse_state_t state) {
    _state = state;
}

bool HTTP_Parser::has_buffered_data() const {
    return _buffer.size() > 0;
}

struct buffer HTTP_Parser::cbuffer() {
    size_t siz = _buffer.size();

    _char_buffer.data = new char[siz];
    std::copy(_buffer.begin(), _buffer.end(), _char_buffer.data);
    _char_buffer.capacity = siz;
    _char_buffer.size = siz;
    return _char_buffer;
}

/* [PARSER
 * CONFIG] */

size_t HTTP_Parser::http_max_method_siz() const {
    return _http_max_method_siz;
}

void HTTP_Parser::set_http_max_method_siz(size_t size) {
    _http_max_method_siz = size;
}

size_t HTTP_Parser::http_max_uri_siz() const {
    return _http_max_uri_siz;
}

void HTTP_Parser::set_http_max_uri_siz(size_t size) {
    _http_max_uri_siz = size;
}

size_t HTTP_Parser::http_max_header_key_siz() const {
    return _http_max_header_key_siz;
}

void HTTP_Parser::set_http_max_header_key_siz(size_t size) {
    _http_max_header_key_siz = size;
}

size_t HTTP_Parser::http_max_header_val_siz() const {
    return _http_max_header_val_siz;
}

void HTTP_Parser::set_http_max_header_val_siz(size_t size) {
    _http_max_header_val_siz = size;
}

size_t HTTP_Parser::http_max_header_amt_siz() const {
    return _http_max_header_amt_siz;
}

void HTTP_Parser::set_http_max_header_amt_siz(size_t size) {
    _http_max_header_amt_siz = size;
}

size_t HTTP_Parser::http_max_body_siz() const {
    return _http_max_body_siz;
}

void HTTP_Parser::set_http_max_body_siz(size_t size) {
    _http_max_body_siz = size;
}

/* [clearing
 * functions] */

void HTTP_Parser::clear() {
    _state = S_PARS_START_CR;

    _token_key.clear();
    _token_val.clear();
    _token.clear();

    _in_dquote = false;
    _escape_next_char = false;
    _rcvd_bytes = 0;
    _expected_bytes = 0;
    _total_expected_bytes = 0;

    _char_buffer = buffer();
}

void HTTP_Parser::reset() {
    clear();
    _buffer.clear();
}

void HTTP_Parser::trim_buffer(size_t pos) {
    if (pos < _buffer.size())
        _buffer.erase(_buffer.begin(), _buffer.begin() + pos);
    else
        _buffer.clear();
}

/* [clearing
 * functions] */

void HTTP_Parser::bufferize(const char* buffer, size_t size) {
    _buffer.insert(_buffer.end(), buffer, buffer + size);
}

/* [HTTP_Parser.read_request()
 */

size_t HTTP_Parser::read_request(HTTP_Request& request) {
    size_t i = 0;

    for (; i < _buffer.size(); ++i) {
        char c = _buffer[i];

    redispatch:
        switch (_state) {
            case S_PARS_START_CR:
                if (IS_CR(c)) {
                    SET_STATE(S_PARS_START_LF);
                } else {
                    GOTO(S_PARS_METHOD);
                }
                break;

            case S_PARS_START_LF:
                if (IS_LF(c)) {
                    SET_STATE(S_PARS_START_CR);
                } else {
                    SET_ERROR;
                }
                break;

            case S_PARS_METHOD:
                if (IS_SP(c)) {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_URI);
                } else if (IS_WSP(c) || IS_CRLF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_NOT_IMPLEMENTED);
                }
                break;

            case S_PARS_URI:
                if (IS_SP(c)) {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_HTTP_NAME);
                } else if (IS_WSP(c) || IS_CRLF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_URI_TOO_LONG);
                }
                break;

            case S_PARS_HTTP_NAME:
                if (c == '/') {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_HTTP_MAJOR);
                } else if (IS_WSP(c) || IS_CRLF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_IM_A_TEAPOT);
                }
                break;

            case S_PARS_HTTP_MAJOR:
                if (c == '.') {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_HTTP_MINOR);
                } else if (IS_WSP(c) || IS_CRLF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_IM_A_TEAPOT);
                }
                break;

            case S_PARS_HTTP_MINOR:
                if (IS_CR(c)) {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_HTTP_MINOR_LF);
                } else if (IS_WSP(c) || IS_LF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_IM_A_TEAPOT);
                }
                break;

            case S_PARS_HTTP_MINOR_LF:
                if (IS_LF(c)) {
                    SET_STATE(S_PARS_HEADER_KEY);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_HEADER_KEY:
                if (c == ':' && _rcvd_header_key_bytes) {
                    SET_STATE(S_PARS_HEADER_OWS_PRE);
                } else if (IS_CR(c)) {
                    SET_STATE(S_PARS_ENDOF_HEADER_LF);
                } else if (!IS_TCHAR(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                }
                break;

            case S_PARS_HEADER_OWS_PRE:
                if (IS_WSP(c)) {
                    continue;
                } else if (IS_CR(c)) {
                    _commit_token(request);
                    SET_STATE(S_PARS_ENDOF_HEADER_LF);
                } else if (IS_VCHAR(c)) {
                    _commit_token(request);
                    ;
                    GOTO(S_PARS_HEADER_VALUE);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_HEADER_VALUE:
                if (IS_DQUOTE(c)) {
                    _in_dquote = !_in_dquote;
                }

                if (IS_CR(c) && !_in_dquote) {
                    SET_STATE(S_PARS_ENDOF_HEADER_LF);
                } else if (IS_WSP(c) && !_in_dquote) {
                    SET_STATE(S_PARS_HEADER_OWS_POST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                }
                break;

            case S_PARS_HEADER_OWS_POST:
                if (IS_WSP(c)) {
                    continue;
                } else if (IS_VCHAR(c) || c == ',') {
                    GOTO(S_PARS_HEADER_OWS_PRE);
                } else if (IS_CR(c)) {
                    SET_STATE(S_PARS_ENDOF_HEADER_LF);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENDOF_HEADER_LF:
                if (!IS_LF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_commit_token(request))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                } else {
                    SET_STATE(S_PARS_BLANK_LINE_CR);
                }
                break;

            case S_PARS_BLANK_LINE_CR:
                if (IS_CR(c)) {
                    SET_STATE(S_PARS_BLANK_LINE_LF);
                } else if (IS_WSP(c) || IS_LF(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else {
                    GOTO(S_PARS_HEADER_KEY);
                }
                break;

            case S_PARS_BLANK_LINE_LF:
                if (IS_LF(c)) {
                    GOTO(S_PARS_INSPECT_REQUEST);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_INSPECT_REQUEST:
                _inspect_request(request);

                if (IS_STATE(S_PARS_HEAD_DONE)) {
                    ++i;
                    GOTO(S_PARS_HEAD_DONE);
                } else if (IS_STATE(S_PARS_ERROR)) {
                    GOTO_ERR;
                } else {
                    continue;
                }

            case S_PARS_RECV_BODY:
                _append_byte(c);

                if (_rcvd_bytes == _expected_bytes) {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_BODY_DONE);
                }
                break;

            /*	transfer-encoding CHUNK body parsing	*/
            case S_PARS_ENCD_DATA:
                GOTO(S_PARS_ENCD_SIZE);

            case S_PARS_ENCD_SIZE:
                if (IS_HEXDIG(c)) {
                    _append_byte(c);
                } else if (IS_CR(c)) {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_ENCD_SIZE_LF);
                } else if (c == ';') {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_ENCD_EXT_START);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_EXT_START:
                if (IS_TCHAR(c)) {
                    GOTO(S_PARS_ENCD_EXT_KEY);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_EXT_KEY:
                if (c == '=' && _rcvd_header_key_bytes) {
                    SET_STATE(S_PARS_ENCD_EXT_VAL);
                } else if (!IS_TCHAR(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                }
                break;

            case S_PARS_ENCD_EXT_VAL:
                if (c == ';') {
                    _commit_token(request);
                    SET_STATE(S_PARS_ENCD_EXT_KEY);
                } else if (IS_CR(c)) {
                    _commit_token(request);
                    SET_STATE(S_PARS_ENCD_EXT_LF);
                } else if (!IS_VCHAR(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                }
                break;

            case S_PARS_ENCD_EXT_LF:
            case S_PARS_ENCD_SIZE_LF:
                if (IS_LF(c)) {
                    GOTO(S_PARS_ENCD_ASSESS_BODY_SIZE);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_ASSESS_BODY_SIZE:
                if (_expected_bytes) {
                    SET_STATE(S_PARS_ENCD_BODY);
                } else {
                    SET_STATE(S_PARS_ENCD_TRAIL_START);
                }
                break;

            case S_PARS_ENCD_BODY:
                _append_byte(c);

                if (_rcvd_bytes == _expected_bytes) {
                    _commit_token(request);
                    IFN_ERR_SET_STATE(S_PARS_ENCD_BODY_CR);
                }
                break;

            case S_PARS_ENCD_BODY_CR:
                if (IS_CR(c)) {
                    SET_STATE(S_PARS_ENCD_BODY_LF);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_BODY_LF:
                if (IS_LF(c)) {
                    SET_STATE(S_PARS_ENCD_SIZE);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_TRAIL_START:
                if (IS_CR(c)) {
                    GOTO(S_PARS_ENCD_END_CR);
                } else if (IS_TCHAR(c)) {
                    GOTO(S_PARS_ENCD_TRAIL_KEY);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_TRAIL_KEY:
                if (c == ':' && _rcvd_header_key_bytes) {
                    SET_STATE(S_PARS_ENCD_TRAIL_VAL);
                } else if (!IS_TCHAR(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                }
                break;

            case S_PARS_ENCD_TRAIL_VAL:
                if (IS_CR(c)) {
                    SET_STATE(S_PARS_ENCD_TRAIL_LF);
                } else if (!IS_VCHAR(c)) {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                } else if (IS_ERR(_append_byte(c))) {
                    REJECT_REQUEST(E_HTTP_HEADER_FIELDS_TOO_LARGE);
                }
                break;

            case S_PARS_ENCD_TRAIL_LF:
                if (IS_LF(c)) {
                    _commit_token(request);
                    SET_STATE(S_PARS_ENCD_TRAIL_START);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_END_CR:
                if (IS_CR(c)) {
                    SET_STATE(S_PARS_ENCD_END_LF);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_ENCD_END_LF:
                if (IS_LF(c)) {
                    SET_STATE(S_PARS_BODY_DONE);
                } else {
                    REJECT_REQUEST(E_HTTP_BAD_REQUEST);
                }
                break;

            case S_PARS_HEAD_DONE:
                request.status = E_HTTP_OK;
                return i;

            case S_PARS_BODY_DONE:
                request.status = E_HTTP_OK;
                return i;

            case S_PARS_ERROR:
                return i;

            default:
                break;
        }
    }
    return i;
}

inline parse_state_t HTTP_Parser::_append_byte(char c) {
    switch (_state) {
        case S_PARS_METHOD:
            if (++_rcvd_bytes <= _http_max_method_siz) {
                _token += c;
            } else {
                SET_ERROR;
            }
            break;

        case S_PARS_URI:
            if (++_rcvd_bytes <= _http_max_uri_siz) {
                _token += c;
            } else {
                SET_ERROR;
            }
            break;

        case S_PARS_HTTP_NAME:
            if (++_rcvd_bytes <= HTTP_NAME_SIZ) {
                _token += c;
            } else {
                SET_ERROR;
            }
            break;

        case S_PARS_HTTP_MAJOR:
            if (++_rcvd_bytes <= INT_MAX_SIZ) {
                _token += c;
            } else {
                SET_ERROR;
            }
            break;

        case S_PARS_HTTP_MINOR:
            if (++_rcvd_bytes <= INT_MAX_SIZ) {
                _token += c;
            } else {
                SET_ERROR;
            }
            break;

        case S_PARS_HEADER_KEY:
            if (++_rcvd_header_key_bytes <= _http_max_header_key_siz) {
                _token_key += c;
            } else {
                SET_ERROR;
            }
            break;

        case S_PARS_HEADER_VALUE:
            if (++_rcvd_header_val_bytes <= _http_max_header_val_siz) {
                _token_val += c;
            } else {
                SET_ERROR;
            }

            break;

        case S_PARS_RECV_BODY:
            ++_rcvd_bytes;
            _token += c;

            break;

        case S_PARS_ENCD_SIZE:
            _token += c;
            ++_rcvd_bytes;

            break;

        case S_PARS_ENCD_EXT_KEY:
            if (++_rcvd_header_key_bytes > _http_max_header_key_siz) {
                SET_ERROR;
            }

            break;

        case S_PARS_ENCD_EXT_VAL:
            if (++_rcvd_header_val_bytes > _http_max_header_val_siz) {
                SET_ERROR;
            }

            break;

        case S_PARS_ENCD_BODY:
            _token += c;
            ++_rcvd_bytes;

            break;

        case S_PARS_ENCD_TRAIL_KEY:
            if (++_rcvd_header_key_bytes > _http_max_header_key_siz) {
                SET_ERROR;
            }

            break;

        case S_PARS_ENCD_TRAIL_VAL:
            if (++_rcvd_header_val_bytes > _http_max_header_val_siz) {
                SET_ERROR;
            }

            break;

        default:
            break;
    }

    return _state;
}

/* [HTTP_Parser._commit_token()
 */

inline parse_state_t HTTP_Parser::_commit_token(HTTP_Request& request) {
    switch (_state) {
        case S_PARS_METHOD:
            request.start_line.method = _token;
            break;

        case S_PARS_URI:
            request.start_line.URL = _token;
            break;

        case S_PARS_HTTP_NAME:
            if (_token == "HTTP") {
                request.start_line.version.name = _token;
            } else {
                SET_ERROR;
            }

            break;

        case S_PARS_HTTP_MAJOR:
            if (char_tools::all_of(_token, ::isdigit)) {
                request.start_line.version.major = std::atoi(_token.c_str());
            } else {
                SET_ERROR;
            }

            break;

        case S_PARS_HTTP_MINOR:
            if (char_tools::all_of(_token, ::isdigit)) {
                request.start_line.version.minor = std::atoi(_token.c_str());
            } else {
                SET_ERROR;
            }

            break;

        case S_PARS_ENDOF_HEADER_LF:
            if (request.headers.size() + 1 > _http_max_header_amt_siz) {
                SET_ERROR;
                break;
            }

            std::transform(_token_key.begin(), _token_key.end(),
                           _token_key.begin(), ::tolower);

            if (request.headers.find(_token_key) != request.headers.end()) {
                request.headers[_token_key] += ", ";
            }
            request.headers[_token_key] += _token_val;

            _rcvd_header_key_bytes = 0;
            _rcvd_header_val_bytes = 0;
            _token_key.clear();
            _token_val.clear();
            break;

        case S_PARS_RECV_BODY:
            request.entity_body.data = _token;
            break;

        case S_PARS_ENCD_TRAIL_LF:
            _rcvd_header_key_bytes = 0;
            _rcvd_header_val_bytes = 0;
            break;

        case S_PARS_ENCD_SIZE:
            if (_token.length() > HEX_MAX_SIZ) {
                request.status = E_HTTP_CONTENT_TOO_LARGE;
                SET_ERROR;
                break;
            }

            char* endptr;
            errno = 0;
            _expected_bytes = strtoul(_token.c_str(), &endptr, HEX_BASE);

            if (errno == ERANGE) {
                request.status = E_HTTP_CONTENT_TOO_LARGE;
                SET_ERROR;
                break;
            } else if (*endptr != '\0') {
                request.status = E_HTTP_BAD_REQUEST;
                SET_ERROR;
                break;
            }

            if ((_total_expected_bytes += _expected_bytes) >
                _http_max_body_siz) {
                request.status = E_HTTP_CONTENT_TOO_LARGE;
                SET_ERROR;
            }
            break;

        case S_PARS_ENCD_EXT_VAL:
            _rcvd_header_key_bytes = 0;
            _rcvd_header_val_bytes = 0;
            break;

        case S_PARS_ENCD_BODY:
            request.entity_body.data += _token;
            _expected_bytes = 0;
            break;

        default:
            break;
    }
    _rcvd_bytes = 0;
    _token.clear();
    return _state;
}

/* [HTTP_Parser._inspect_request()
 */

inline parse_state_t HTTP_Parser::_inspect_request(HTTP_Request& request) {
    int major = request.start_line.version.major;
    int minor = request.start_line.version.minor;
    unsigned int host_count = 0;
    unsigned int encode_count = 0;
    unsigned int length_count = 0;

    {
        if (request.start_line.version.name != "HTTP" || major != 1 ||
            minor != 1) {
            request.status = E_HTTP_BAD_REQUEST;
            return SET_ERROR;
        }
    }

    {
        HTTP_Request::header_cit_t cur_header = request.headers.begin();
        HTTP_Request::header_cit_t last_header = request.headers.end();

        for (; cur_header != last_header; ++cur_header) {
            if (cur_header->first == "host") {
                ++host_count;
            };
            if (cur_header->first == "content-length") {
                ++length_count;
            };
            if (cur_header->first == "transfer-encoding") {
                ++encode_count;
            };

            if (cur_header->second.find(',') != std::string::npos) {
                const http_header_t* header =
                    http_header(cur_header->first.c_str());

                if (header) {
                    if (header->type == HEAD_SINGLETON) {
                        request.status = E_HTTP_BAD_REQUEST;
                        return SET_ERROR;
                    }
                }
            }
        }
    }

    {
        bool length_validity = length_count <= 1;
        bool host_validity = host_count <= 1;

        if (!length_validity || !host_validity) {
            request.status = E_HTTP_BAD_REQUEST;
            return SET_ERROR;
        }

        if (host_count == 0) {
            bool host_count_compliance =
                (major < 1 || (major == 1 && minor == 0));

            if (!host_count_compliance) {
                request.status = E_HTTP_BAD_REQUEST;
                return SET_ERROR;
            }
        }

        if (length_count + encode_count > 1) {
            request.status = E_HTTP_BAD_REQUEST;
            return SET_ERROR;
        }
    }

    {
        if (length_count) {
            HTTP_Request::header_it_t hd =
                request.headers.find("content-length");

            if (hd->second.find(',') != std::string::npos) {
                request.status = E_HTTP_BAD_REQUEST;
                return SET_ERROR;
            }

            if (!char_tools::all_of(hd->second, ::isdigit)) {
                request.status = E_HTTP_BAD_REQUEST;
                return SET_ERROR;
            } else {
                const char* val = hd->second.c_str();
                char* endptr = NULL;

                errno = 0;
                request.entity_body.size = strtoul(val, &endptr, DEC_BASE);

                if (errno == ERANGE ||
                    request.entity_body.size > _http_max_body_siz) {
                    request.status = E_HTTP_CONTENT_TOO_LARGE;
                    return SET_ERROR;
                } else if (*endptr != '\0') {
                    request.status = E_HTTP_BAD_REQUEST;
                    return SET_ERROR;
                } else {
                    _expected_bytes = request.entity_body.size;
                    request.body_transfer_typ = LENGTH_TRANSF;
                    return SET_STATE(S_PARS_HEAD_DONE);
                }
            }
        } else if (encode_count) {
            request.body_transfer_typ = CHUNKED_TRANSF;
            return SET_STATE(S_PARS_HEAD_DONE);
        } else {
            return SET_STATE(S_PARS_HEAD_DONE);
        }
    }
}
