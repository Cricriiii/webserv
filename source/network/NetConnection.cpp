#include "network/NetConnection.hpp"

#include <signal.h>
#include <sys/types.h>

#include "network/SessionManager.hpp"
#include "tools/KeyVal_Parser.hpp"
#include "tools/char_tools.hpp"
#include "tools/hexdump.hpp"

/* [CONSTRUCTORS
 */

NetConnection::NetConnection(int socketd, const conf_http_t& http_conf,
                             size_t obuf_siz)
    : _socketd(socketd),
      _poll_state(S_POLL_NONE),
      _state(S_CONN_WAIT),
      _state_change(false),
      _request(socketd),
      _response(),
      _parser(http_conf),
      _serializer(obuf_siz),
      _context(),
      _keep_alive(false),
      _kill(false),
      _handler(NULL),
      _resource(),
      _total_payload_progress(0),
      _in_ready_list(false),
      _session(NULL),
      _is_new_session(false),
      _cgi_start_time(0),
      _wait_before_next_io(true) {
}

NetConnection::NetConnection(const NetConnection& other)
    : _socketd(other._socketd),
      _poll_state(other._poll_state),
      _state(other._state),
      _state_change(other._state_change),
      _request(other._request),
      _response(other._response),
      _parser(other._parser),
      _serializer(other._serializer),
      _context(other._context),
      _keep_alive(other._keep_alive),
      _kill(other._kill),
      _handler(other._handler),
      _resource(other._resource),
      _total_payload_progress(other._total_payload_progress),
      _in_ready_list(other._in_ready_list),
      _session(other._session),
      _is_new_session(other._is_new_session),
      _cgi_start_time(other._cgi_start_time),
      _wait_before_next_io(other._wait_before_next_io) {
}

NetConnection::~NetConnection() {
    delete _handler;

    clear_resource();
}

NetConnection& NetConnection::operator=(const NetConnection& other) {
    if (this != &other) {
        _socketd = other._socketd;
        _poll_state = other._poll_state;
        _state = other._state;
        _state_change = other._state_change;
        _request = other._request;
        _response = other._response;
        _parser = other._parser;
        _serializer = other._serializer;
        _context = other._context;
        _keep_alive = other._keep_alive;
        _kill = other._kill;
        _handler = other._handler;
        _resource = other._resource;
        _total_payload_progress = other._total_payload_progress;
        _in_ready_list = other._in_ready_list;
        _session = other._session;
        _is_new_session = other._is_new_session;
        _cgi_start_time = other._cgi_start_time;
        _wait_before_next_io = other._wait_before_next_io;
    }
    return *this;
}

/* [READ
 * REQUEST
 */

bool NetConnection::_retrieve_session() {
    HTTP_Request::cookie_cit_t sid_cookie = _request.cookies.find("session_id");

    if (sid_cookie != _request.cookies.end()) {
        SessionManager::Session* session =
            SessionManager::get_instance().get_session(sid_cookie->second);
        return _request.session = session, _session = session;
    } else {
        return false;
    }
}

SessionManager::Session* NetConnection::_init_session() {
    if (_session == NULL) {
        std::string sid = SessionManager::get_instance().generate_session_id();
        SessionManager::get_instance().add_session(sid);
        _session = SessionManager::get_instance().get_session(sid);
        _is_new_session = true;
    }
    return _session;
}

size_t NetConnection::read_request(const char* buffer, size_t size) {
    if (ENABLE_IO_HEXDUMP)
        hexdump(buffer, size);
    _parser.bufferize(buffer, size);
    return read_request();
}

size_t NetConnection::read_request() {
    size_t consumed = _parser.read_request(_request);
    parse_state_t parse_state = _parser.state();

    switch (parse_state) {
        case S_PARS_HEAD_DONE:
            _request.extract_cookies();
            _retrieve_session();

            set_state(S_CONN_HANDLE);
            break;

        case S_PARS_BODY_DONE:
            set_state(S_CONN_BUILD_HEAD);
            break;

        case S_PARS_ERROR:
            set_state(S_CONN_HANDLE);
            break;

        default:
            if (!_IS_READ_ENCD_BODY(parse_state)) {
                set_state(S_CONN_WAIT);
            }
            break;
    }

    _parser.trim_buffer(consumed);
    return consumed;
}

/* [HANDLE
 * REQUEST]
 */

void NetConnection::_define_close_behavior() {
    if (HTTP_IS_ERR(_request.status)) {
        _kill = true;
    } else {
        HTTP_Request::header_it_t connection =
            _request.headers.find("connection");
        if (connection != _request.headers.end()) {
            _keep_alive = (connection->second == "keep-alive" ? true : false);
        } else {
            _keep_alive = false;
        }
    }
}

void NetConnection::_execute_request() {
    try {
        _handler = HandlerValidator::validator(_context);
        if (_handler) {
            _resource = _handler->execute(_context);
            delete _handler;
            _handler = NULL;
        }
    } catch (std::bad_alloc& e) {
        set_state(S_CONN_CLI_CLOSE);
    }
}

void NetConnection::feed_cgi() {
    ssize_t written = 0;

    if (_request.body_transfer_typ == LENGTH_TRANSF) {
        buffer buf = _parser.cbuffer();
        _total_payload_progress = 0;

        written = write(
            _resource.file_cgi_write.fd, buf.data + _total_payload_progress,
            _resource.file_cgi_write.size - _total_payload_progress);

        if (written < 0) {
            return;
        }
        _total_payload_progress += written;

        _parser.trim_buffer(_total_payload_progress);

        if (_total_payload_progress >= _resource.file_cgi_write.size) {
            set_state(S_CONN_CGI_WAIT);
            close(_resource.file_cgi_write.fd),
                _resource.file_cgi_write.fd = -1;
        }

        delete[] buf.data;
    } else if (_request.body_transfer_typ == CHUNKED_TRANSF) {
        written +=
            write(_resource.file_cgi_write.fd,
                  _request.entity_body.data.c_str() + _total_payload_progress,
                  _request.entity_body.data.size() - _total_payload_progress);

        if (written < 0) {
            return;
        }
        _total_payload_progress += written;

        _request.entity_body.data.erase(0, _total_payload_progress);
        _total_payload_progress = 0;
    }
}

void NetConnection::_flush_body_into_file() {
    ssize_t written = 0;

    if (_request.body_transfer_typ == LENGTH_TRANSF) {
        buffer buf = _parser.cbuffer();

        size_t to_copy =
            static_cast<ssize_t>(_resource.file_write.size) > buf.size
                ? buf.size
                : _resource.file_write.size;

        _total_payload_progress = 0;
        written =
            write(_resource.file_write.fd, buf.data + _total_payload_progress,
                  to_copy - _total_payload_progress);

        if (written < 0) {
            return;
        }
        _total_payload_progress += written;

        _parser.trim_buffer(_total_payload_progress);

        if (_total_payload_progress >= to_copy) {
            if (_total_payload_progress >= _resource.file_write.size) {
                close(_resource.file_write.fd), _resource.file_write.fd = -1;
                set_state(S_CONN_BUILD_HEAD);
            } else {
                set_state(S_CONN_DLOAD_BODY);
            }
            delete[] buf.data;
        }
    } else if (_request.body_transfer_typ == CHUNKED_TRANSF) {
        written +=
            write(_resource.file_write.fd,
                  _request.entity_body.data.c_str() + _total_payload_progress,
                  _request.entity_body.data.size() - _total_payload_progress);

        if (written < 0) {
            return;
        }
        _total_payload_progress += written;
        _request.entity_body.data.erase(0, _total_payload_progress);
        _total_payload_progress = 0;
    }
}

void NetConnection::_flush_post_header_buffer() {
    if (!_parser.has_buffered_data()) {
        return;
    }

    switch (_request.body_transfer_typ) {
        case LENGTH_TRANSF:
            _flush_body_into_file();
            break;

        case CHUNKED_TRANSF:
            _parser.set_state(S_PARS_ENCD_DATA);
            read_request();
            _flush_body_into_file();
            break;

        case EMPTY_TRANSF:
            throw std::runtime_error(
                "NetConnection: Unexpected state in handle_request write path");
    }
}

void NetConnection::handle_request() {
    _execute_request();
    _define_close_behavior();

    if (_resource.file_cgi_write.fd != -1) {
        set_state(S_CONN_WRITE_TO_CGI);
        return;
    }

    if (_resource.file_cgi_read.fd != -1) {
        set_state(S_CONN_CGI_WAIT);
    } else if (_resource.file_write.fd != -1) {
        set_state(S_CONN_DLOAD_BODY);
        _flush_post_header_buffer();
    } else {
        set_state(S_CONN_BUILD_HEAD);
    }
}

/* [SAVE
 * BODY
 */

bool NetConnection::_save_content_body(char* buffer, ssize_t* buffer_size) {
    ssize_t payload_progress = 0;

    if (*buffer_size > 0) {
        size_t to_copy;
        if (_total_payload_progress + *buffer_size <=
            _resource.file_write.size) {
            to_copy = *buffer_size;
        } else {
            to_copy = _resource.file_write.size - _total_payload_progress;
        }
        payload_progress += write(_resource.file_write.fd, buffer, to_copy);
    }

    if (payload_progress > 0) {
        _total_payload_progress += payload_progress;
        std::copy(buffer + payload_progress, buffer + *buffer_size, buffer);
        *buffer_size -= payload_progress;
    }

    if (_total_payload_progress >= _resource.file_write.size) {
        _parser.bufferize(buffer, *buffer_size);
        set_state(S_CONN_BUILD_HEAD);
        close(_resource.file_write.fd), _resource.file_write.fd = -1;
        return true;
    } else {
        return false;
    }
}

bool NetConnection::_save_chunk_body(char* buffer, ssize_t* buffer_size) {
    ssize_t payload_progress = 0;
    size_t consumed;

    if (_parser.state() == S_PARS_HEAD_DONE) {
        _parser.set_state(S_PARS_ENCD_DATA);
    }

    if (*buffer_size > 0) {
        consumed = read_request(buffer, *buffer_size);

        size_t to_copy = _request.entity_body.data.size();

        while (payload_progress <
               static_cast<ssize_t>(_request.entity_body.data.size())) {
            payload_progress = write(
                _resource.file_write.fd,
                _request.entity_body.data.c_str() + payload_progress, to_copy);

            if (payload_progress > 0) {
                to_copy -= payload_progress;
            }
        }
        *buffer_size = payload_progress;
    }

    _request.entity_body.data.erase(0, payload_progress);

    if (_parser.state() == S_PARS_BODY_DONE) {
        set_state(S_CONN_BUILD_HEAD);
        close(_resource.file_write.fd), _resource.file_write.fd = -1;
        return true;
    } else {
        return false;
    }
}

bool NetConnection::save_body(char* buffer, ssize_t* buffer_size) {
    switch (_request.body_transfer_typ) {
        case LENGTH_TRANSF:
            return _save_content_body(buffer, buffer_size);
        case CHUNKED_TRANSF:
            return _save_chunk_body(buffer, buffer_size);
        case EMPTY_TRANSF:
            return true;
        default:
            return _request.status = E_HTTP_NOT_IMPLEMENTED, true;
    }
}

/* [BUILD
 * HEADERS
 */

void NetConnection::_add_header_if_session() {
    if (_is_new_session && _session) {
        _is_new_session = false;

        std::string cookie_val;

        cookie_val = "session_id=";
        cookie_val += _session->id;
        _create_header("Set-Cookie", cookie_val.c_str());

        if (HTTP_ONLY) {
            _append_header("Set-Cookie", "HttpOnly");
        }

        _append_header("Set-Cookie", "Path=/");
    }
}

void NetConnection::_add_header_if_length() {
    size_t siz = 0;

    if (_resource.file_cgi_read.fd != -1) {
        HTTP_Request::header_cit_t cit = char_tools::map_string_find_nocase(
            _response.headers, "content-length");

        if (cit != _response.headers.end()) {
            char* end = NULL;
            siz = std::strtol(cit->second.c_str(), &end, DEC_BASE);
            if (*end != '\0')
                siz = 0;
        }

        if (siz == 0) {
            siz = cit->second.size();
        }

    } else if (_resource.file_read.fd != -1) {
        siz = _resource.file_read.size + 2;
    } else if (_resource.file_tmp.fd != -1) {
        siz = _resource.file_tmp.size + 2;
    }

    if (siz) {
        std::string siz_str;
        StringHandler sh(siz_str);
        sh += siz;
        _create_header("Content-length", siz_str.c_str());
    }
}

__attribute__((nonnull)) void NetConnection::_create_header(const char* key,
                                                            const char* val) {
    _response.headers[key] = val;
}

__attribute__((nonnull)) void NetConnection::_append_header(const char* key,
                                                            const char* val) {
    _response.headers[key] += ';';
    _response.headers[key] += val;
}

void NetConnection::build_headers() {
    _create_header("Server", APP_NAME);
    _create_header("Connection",
                   _keep_alive && !_kill ? "keep-alive" : "close");

    _add_header_if_session();
    _add_header_if_length();

    if (!_resource.mime_type.empty())
        _create_header("Content-type", _resource.mime_type.c_str());
}

/* [COMMON GATEWAY
 * INTERFACE
 */

bool NetConnection::is_cgi() const {
    return _resource.cgi.process > 0;
}

int NetConnection::wait_cgi() {
    int retval = 0;
    pid_t pid = ::waitpid(_resource.cgi.process, &retval, WNOHANG);

    switch (pid) {
        case -1:
            return CGI_WAIT_ERR;

        case 0:
            return CGI_EXEC_ON;

        default:
            if (WIFEXITED(retval) && WEXITSTATUS(retval) == 0) {
                return CGI_EXIT_OK;
            } else {
                return CGI_EXEC_FAIL;
            }
    }
}

long long NetConnection::cgi_start_time() const {
    return _resource.cgi.start_cgi;
}

int NetConnection::kill_cgi() {
    int op = 0;
    if (_resource.cgi.process != -1) {
        op = ::kill(_resource.cgi.process, SIGKILL);
        _resource.cgi.process = -1;

        if (_resource.file_cgi_read.fd != -1)
            ::close(_resource.file_cgi_read.fd);
        if (_resource.file_cgi_write.fd != -1)
            ::close(_resource.file_cgi_write.fd);

        return op;
    }
    return op;
}

void NetConnection::_try_new_session(HTTP_Request::header_t& header_list) {
    HTTP_Request::header_t::iterator it_login =
        char_tools::map_string_find_nocase(header_list, "New-Login");

    if (it_login != header_list.end()) {
        if (!SessionManager::get_instance().retrieve_login(it_login->second)) {
            SessionManager::Session* cur_session = _init_session();

            cur_session->values.insert(
                std::make_pair("login", it_login->second));
        }
    }
}

void NetConnection::_parse_cgi_headers() {
    size_t pos = _response.entity_body.data.find("\r\n\r\n");
    std::string headers = _response.entity_body.data.substr(0, pos);

    if (headers.size()) {
        HTTP_Request::header_t header_list = KeyVal_Parser::build_dictionary(
            headers, ":", "\r\n", CHARSET, LITERAL);

        _try_new_session(header_list);

        for (std::map<std::string, std::string>::iterator it =
                 header_list.begin();
             it != header_list.end(); ++it) {
            _response.headers.insert(std::make_pair(it->first, it->second));
        }

        size_t offset = pos + 4; /*	pos + \r\n\r\n	*/
        _response.entity_body.data.erase(
            _response.entity_body.data.begin(),
            _response.entity_body.data.begin() + offset);
    }
}

bool NetConnection::read_cgi_output(char* buffer, ssize_t* buffer_read,
                                    size_t buffer_capacity) {
    if (wait_before_next_io()) {
        set_wait_before_next_io(false);
        return false;
    } else {
        set_wait_before_next_io(true);

        if ((*buffer_read = read(_resource.file_cgi_read.fd, buffer,
                                 buffer_capacity)) > 0) {
            _response.entity_body.data.append(buffer, *buffer_read);
            return false;
        } else if (*buffer_read == 0) {
            _parse_cgi_headers();
            return true;
        } else {
            return false;
        }
    }
}

void NetConnection::set_server_err(e_http_status error_status) {
    _request.status = error_status;
}

/* [SERIALIZED OUT
 * STREAMS
 */

response_status_t NetConnection::head_stream(char* buffer, ssize_t capacity) {
    return _serializer.serialize_head(_response, _resource, buffer, capacity);
}

response_status_t NetConnection::body_stream(char* buffer, ssize_t capacity) {
    return _serializer.serialize_body(_response, _resource, buffer, capacity);
}

void NetConnection::sync_cursor(ssize_t bytes_sent, size_t& cur) {
    _serializer.sync_cursor(bytes_sent, cur);
}

/* [CONFIG -
 * PARSER
 */

size_t NetConnection::http_max_method_siz() const {
    return _parser.http_max_method_siz();
}

void NetConnection::set_http_max_method_siz(size_t size) {
    _parser.set_http_max_method_siz(size);
}

size_t NetConnection::http_max_uri_siz() const {
    return _parser.http_max_uri_siz();
}

void NetConnection::set_http_max_uri_siz(size_t size) {
    _parser.set_http_max_uri_siz(size);
}

size_t NetConnection::http_max_header_key_siz() const {
    return _parser.http_max_header_key_siz();
}

void NetConnection::set_http_max_header_key_siz(size_t size) {
    _parser.set_http_max_header_key_siz(size);
}

size_t NetConnection::http_max_header_val_siz() const {
    return _parser.http_max_header_val_siz();
}

void NetConnection::set_http_max_header_val_siz(size_t size) {
    _parser.set_http_max_header_val_siz(size);
}

size_t NetConnection::http_max_header_amt_siz() const {
    return _parser.http_max_header_amt_siz();
}

void NetConnection::set_http_max_header_amt_siz(size_t size) {
    _parser.set_http_max_header_amt_siz(size);
}

size_t NetConnection::http_max_body_siz() const {
    return _parser.http_max_body_siz();
}

void NetConnection::set_http_max_body_siz(size_t size) {
    _parser.set_http_max_body_siz(size);
}

bool NetConnection::ready() const {
    return _in_ready_list;
}

void NetConnection::set_readiness(bool state) {
    _in_ready_list = state;
}

IHandler* NetConnection::handler() {
    return _handler;
}

bool NetConnection::wait_before_next_io() const {
    return _wait_before_next_io;
}

void NetConnection::set_wait_before_next_io(bool state) {
    _wait_before_next_io = state;
}

/* [CONFIG -
 * SERIALIZER
 */

void NetConnection::set_send_segment_siz(size_t size) {
    _serializer.set_send_segment_siz(size);
}

/* [CLEARING
 * FUNCTIONS
 */

void NetConnection::clear_request() {
    _request.clear();
}

void NetConnection::clear_response() {
    _response.clear();
}

void NetConnection::clear_parser() {
    _parser.clear();
}

void NetConnection::clear_resource() {
    if (_resource.cgi.process != -1) {
        waitpid(_resource.cgi.process, NULL, WNOHANG);
        _resource.cgi.process = -1;
    }

    if (_resource.file_read.fd != -1) {
        ::close(_resource.file_read.fd);
        GarbageCollector::get_instance().remove_fd(_resource.file_read.fd);
        if (_resource.file_read.name_file.length()) {
            unlink(_resource.file_read.name_file.c_str());
        }
    }
    if (_resource.file_write.fd != -1) {
        ::close(_resource.file_write.fd);
        GarbageCollector::get_instance().remove_fd(_resource.file_write.fd);
        if (_resource.file_write.name_file.length()) {
            unlink(_resource.file_write.name_file.c_str());
        }
    }
    if (_resource.file_tmp.fd != -1) {
        ::close(_resource.file_tmp.fd);
        GarbageCollector::get_instance().remove_fd(_resource.file_tmp.fd);
        if (_resource.file_tmp.name_file.length()) {
            unlink(_resource.file_tmp.name_file.c_str());
        }
    }
    if (_resource.file_cgi_read.fd != -1) {
        ::close(_resource.file_cgi_read.fd);
        GarbageCollector::get_instance().remove_fd(_resource.file_cgi_read.fd);
        if (_resource.file_cgi_read.name_file.length()) {
            unlink(_resource.file_cgi_read.name_file.c_str());
        }
    }
    if (_resource.file_cgi_write.fd != -1) {
        ::close(_resource.file_cgi_write.fd);
        GarbageCollector::get_instance().remove_fd(_resource.file_cgi_write.fd);
        if (_resource.file_cgi_write.name_file.length()) {
            unlink(_resource.file_cgi_write.name_file.c_str());
        }
    }
}

void NetConnection::clear() {
    _request.clear();
    _response.clear();
    _parser.clear();
    _serializer.clear();
}

void NetConnection::reset() {
    clear_resource();
    _request.clear();
    _response.clear();
    _parser.clear();
    _serializer.reset();
    clear_resource();
    _session = NULL;
    _is_new_session = false;
    _total_payload_progress = 0;
    _state_change = false;
}

/* [GENERIC GETTERS /
 * SETTERS
 */

int NetConnection::socketd() const {
    return _socketd;
}

void NetConnection::set_poll_state(poll_state_t state) {
    _poll_state = state;
}

poll_state_t NetConnection::poll_state() {
    return _poll_state;
}

void NetConnection::set_state(conn_state_t state) {
    _state_change = true;
    _state = state;
}

bool NetConnection::state_change() {
    return _state_change;
}

void NetConnection::reset_state_change() {
    _state_change = false;
}

conn_state_t NetConnection::state() {
    return _state;
}

const HTTP_Request& NetConnection::request() const {
    return _request;
}

const HTTP_Response& NetConnection::response() const {
    return _response;
}

bool NetConnection::ibuffer_isdata(void) const {
    return _parser.has_buffered_data();
}

bool NetConnection::keep_alive() const {
    return _keep_alive;
}

bool NetConnection::kill() const {
    return _kill;
}

HTTP_Request& NetConnection::request() {
    return _request;
}

HTTP_Response& NetConnection::response() {
    return _response;
}

void NetConnection::load_context(const request_context_t& context) {
    _context = context;
}
