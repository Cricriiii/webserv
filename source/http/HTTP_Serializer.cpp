#include "http/HTTP_Serializer.hpp"

HTTP_Serializer::HTTP_Serializer(size_t buf_siz)
    : _pos(), _hd_idx(), _state(), _cur_file(), _buffer() {
    _buffer.reserve(buf_siz);
}

HTTP_Serializer::HTTP_Serializer(const HTTP_Serializer& other)
    : _pos(other._pos),
      _hd_idx(other._hd_idx),
      _state(other._state),
      _cur_file(other._cur_file),
      _buffer(other._buffer) {
}

HTTP_Serializer::~HTTP_Serializer() {
}

HTTP_Serializer& HTTP_Serializer::operator=(const HTTP_Serializer& other) {
    if (this != &other) {
        _pos = other._pos;
        _hd_idx = other._hd_idx;
        _state = other._state;
        _cur_file = other._cur_file;
        _buffer = other._buffer;
    }
    return *this;
}

void HTTP_Serializer::set_send_segment_siz(size_t size) {
    _buffer.resize(size);
}

void HTTP_Serializer::clear() {
    _pos = 0;
    _buffer.clear();
}

void HTTP_Serializer::reset() {
    clear();
    _hd_idx = 0;
    _cur_file = file_context();
    _state = SET_CUR_FILE;
}

response_status_t HTTP_Serializer::serialize_head(const HTTP_Response& response,
                                                  resource_context_t& resource,
                                                  char* target,
                                                  size_t target_size) {
    StringHandler str_hdlr(_buffer);
    size_t to_copy = 0;
    size_t total_copied = 0;
    bool goto_next_step = true;
    HTTP_Response::header_it_t h_it;
    response_status_t response_status;

redispatch:
    switch (_state) {
        case SET_CUR_FILE:

            if (resource.file_tmp.fd != -1) {
                _cur_file = resource.file_tmp;
            } else if (resource.file_read.fd != -1) {
                _cur_file = resource.file_read;
            } else if (resource.file_cgi_read.fd != -1) {
                _cur_file = resource.file_cgi_read;
            }

            _state = BUILD_START_LINE;

        case BUILD_START_LINE:
            str_hdlr += response.start_line.version.name;
            str_hdlr += "/";
            str_hdlr += 1;
            str_hdlr += ".";
            str_hdlr += 1;
            str_hdlr += " ";
            str_hdlr += resource.http.status;
            str_hdlr += " ";
            str_hdlr += resource.http.reason;
            str_hdlr += "\r\n";

            _state = SEND_START_LINE;
            goto redispatch;

        case SEND_START_LINE:

            to_copy = _buffer.size() - _pos;
            if (to_copy >= target_size) {
                to_copy = target_size;
                goto_next_step = false;
            }

            std::copy(_buffer.begin() + _pos, _buffer.begin() + _pos + to_copy,
                      target + total_copied);
            total_copied += to_copy;
            _pos += total_copied;

            if (goto_next_step) {
                _state = BUILD_HEADERS;
                _buffer.clear();
                _pos = 0;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case BUILD_HEADERS:

            for (HTTP_Response::header_cit_t cit = response.headers.begin();
                 cit != response.headers.end(); ++cit) {
                str_hdlr += cit->first;
                str_hdlr += ":";
                str_hdlr += cit->second;
                str_hdlr += "\r\n";
            }

            str_hdlr += "\r\n";

            _state = SEND_HEADERS;
            goto redispatch;

        case SEND_HEADERS:

            to_copy = _buffer.size();
            if (to_copy >= target_size) {
                to_copy = target_size - total_copied;
                goto_next_step = false;
            }

            std::copy(_buffer.begin() + _pos, _buffer.begin() + _pos + to_copy,
                      target + total_copied);
            _buffer.erase(0, to_copy);
            total_copied += to_copy;
            _pos = 0;

            if (goto_next_step) {
                _state = PRE_BUILD_BODY;
                _buffer.clear();
                _pos = 0;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case PRE_BUILD_BODY:
            response_status.cur = total_copied;
            response_status.done = true;
            response_status.fd = _cur_file.fd;

            if (resource.file_cgi_read.fd != -1) {
                _state = BUILD_CGI_BODY;
            } else {
                _state = BUILD_REG_BODY;
            }

            return response_status;

        default:
            throw std::runtime_error("Unexpected state in serialize_head");
    }
}

response_status_t HTTP_Serializer::serialize_body(const HTTP_Response& response,
                                                  resource_context_t& resource,
                                                  char* target,
                                                  size_t target_size) {
    (void)resource;

    size_t to_copy = 0;
    size_t total_copied = 0;
    ssize_t read_bytes = 0;
    bool goto_next_step = true;
    response_status_t response_status;

redispatch:
    switch (_state) {
        case BUILD_CGI_BODY:
            to_copy = response.entity_body.data.size() - _pos;

            if (to_copy == 0) {
                _state = SERIAL_DONE;
                goto redispatch;
            } else if (to_copy > target_size) {
                to_copy = target_size;
                goto_next_step = false;
            }

            std::copy(response.entity_body.data.begin() + _pos,
                      response.entity_body.data.begin() + _pos + to_copy,
                      target + total_copied);
            total_copied += to_copy;
            _pos += total_copied;

            if (goto_next_step) {
                _state = SEND_CGI_BODY;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case BUILD_REG_BODY:
            to_copy = _cur_file.size - _pos;

            if (to_copy == 0) {
                _state = SERIAL_DONE;
                goto redispatch;
            } else if (to_copy > target_size) {
                to_copy = target_size;
                goto_next_step = false;
            }

            read_bytes = read(_cur_file.fd, target + total_copied, to_copy);
            if (read_bytes > 0) {
                total_copied += read_bytes;
                _pos += read_bytes;
            }

            if (goto_next_step) {
                _state = SEND_REG_BODY;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case SEND_CGI_BODY:
            if (_pos == response.entity_body.data.size()) {
                _state = SEND_LAST_CR;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case SEND_REG_BODY:
            if (_pos == _cur_file.size) {
                _state = SEND_LAST_CR;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case SEND_LAST_CR:
            if (target_size - total_copied > 0) {
                target[total_copied++] = '\r';
                _state = SEND_LAST_LF;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case SEND_LAST_LF:
            if (target_size - total_copied > 0) {
                target[total_copied++] = '\n';
                _state = SERIAL_DONE;
                goto redispatch;
            } else {
                response_status.cur = total_copied;
                response_status.done = false;
                response_status.fd = _cur_file.fd;
                return response_status;
            }

        case SERIAL_DONE:
            response_status.cur = total_copied;
            response_status.done = true;
            response_status.fd = _cur_file.fd;
            return response_status;

        default:
            throw std::runtime_error("Unexpected state in serialize_body");
    }
}

void HTTP_Serializer::sync_cursor(ssize_t bytes_sent, size_t& cur) {
    if (bytes_sent > 0 && static_cast<size_t>(bytes_sent) != cur) {
        _pos -= bytes_sent;
        cur = _pos;
    }
}
