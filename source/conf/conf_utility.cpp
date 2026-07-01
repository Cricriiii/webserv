#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "conf.hpp"
#include "tools/char_tools.hpp"

const std::string Server_configuration::msg_error(const std::string& error,
                                                  int index) const {
    if (index < 0)
        return ".conf: " + error;

    return ".conf: " + error +
           " at line: " + char_tools::itostr(current_line(index));
}

int Server_configuration::current_line(int index) const {
    if (_tokens.empty())
        return 0;
    if (index < 0)
        return _tokens[0].line;

    if (static_cast<size_t>(index) >= _tokens.size())
        return _tokens[_tokens.size() - 1].line;

    return _tokens[index].line;
}

bool Server_configuration::verif_server_conflict(vserver_t& server) {
    vservers_it_t it = _vservers.begin();
    vservers_it_t ite = _vservers.end();

    for (; it != ite; ++it) {
        if (it->port == server.port && it->name == server.name)
            return true;
    }
    return false;
}

int Server_configuration::strtoi(const std::string& s, int index) const {
    int out;
    char* end;

    if (s.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument(msg_error("expected number", index));

    errno = 0;
    end = 0;
    out = std::strtol(s.c_str(), &end, 10);

    if (end == s.c_str() || *end != '\0')
        throw std::invalid_argument(msg_error("invalid number", index));
    if (errno == ERANGE || out > INT_MAX)
        throw std::invalid_argument(msg_error("number too high", index));
    if (out < 0)
        throw std::invalid_argument(
            msg_error("number must be positive", index));
    return static_cast<int>(out);
}

long long Server_configuration::strtoll(const std::string& s, int index) const {
    long long out;
    char* end;

    if (s.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument(msg_error("expected number", index));

    errno = 0;
    end = 0;
    out = std::strtoll(s.c_str(), &end, 10);

    if (end == s.c_str() || *end != '\0')
        throw std::invalid_argument(msg_error("invalid number", index));
    if (errno == ERANGE || out > LLONG_MAX)
        throw std::invalid_argument(msg_error("number too high", index));
    if (out < 0)
        throw std::invalid_argument(
            msg_error("number must be positive", index));
    return static_cast<long long>(out);
}

bool Server_configuration::verif_location_path(vserver_t& server,
                                               const std::string& path) const {
    for (size_t i = 0; i < server.location.size(); ++i) {
        if (server.location[i].path == path)
            return true;
    }
    return false;
}

void Server_configuration::init_vserver(vserver_t& server) {
    server.name.clear();
    server.port = -1;
    server.max_body_size = -1;
    server.root = "";
    server.index.clear();
    server.errors.clear();
    server.location.clear();
}

void Server_configuration::init_conf_http() {
    _conf_http.http_max_body_siz = -1;
    _conf_http.recv_segment_siz = -1;
    _conf_http.send_segment_siz = -1;
    _conf_http.http_max_method_siz = -1;
    _conf_http.http_max_uri_siz = -1;
    _conf_http.http_max_header_key_siz = -1;
    _conf_http.http_max_header_val_siz = -1;
    _conf_http.http_max_header_amt_siz = -1;
}

bool Server_configuration::is_end_token(size_t index) const {
    return (index >= _tokens.size());
}

bool Server_configuration::is_token(size_t index,
                                    const std::string& value) const {
    return (index < _tokens.size() && _tokens[index].token == value);
}

const std::string& Server_configuration::get_token(size_t index) const {
    return _tokens[index].token;
}

bool Server_configuration::has_status(size_t index,
                                      e_status_token status) const {
    return (index < _tokens.size() && _tokens[index].status == status);
}

char Server_configuration::current_char() const {
    if (_i_file < _file.size())
        return _file[_i_file];
    return '\0';
}
