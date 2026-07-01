#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <stdexcept>

#include "conf.hpp"

#define BUFFER_SIZE 4096

static bool space(char c) {
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        return true;
    return false;
}

static bool quote(char c) {
    if (c == '\"' || c == '\'')
        return true;
    return false;
}

static bool end_line(char c) {
    if (c == ';' || c == '}')
        return true;
    return false;
}

static bool directive(char c) {
    if (c == '{' || c == ';' || c == '}')
        return true;
    return false;
}

void Server_configuration::open_read_file(const char* path_file) {
    if (access(path_file, F_OK) == -1)
        throw std::runtime_error(msg_error("file not found", -1));

    int fd_conf = open(path_file, O_RDONLY);
    if (fd_conf == -1)
        throw std::runtime_error(msg_error("open file", -1));

    while (1) {
        char buffer[BUFFER_SIZE];
        ssize_t octet = read(fd_conf, buffer, BUFFER_SIZE);
        if (octet == -1)
            throw std::runtime_error(msg_error("read file", -1));
        if (octet == 0)
            break;

        _file.append(buffer, octet);
    }

    close(fd_conf);
    if (_file.size() == 0)
        throw std::runtime_error(msg_error("file is empty", -1));
}

void Server_configuration::lexer() {
    while (_i_file < _file.size()) {
        if (space(current_char()))
            cut_space();

        else if (current_char() == '#')
            comment();

        else if (quote(current_char()))
            token_quote();

        else if (directive(current_char()))
            token_directive();

        else
            cut_word();
    }
}

void Server_configuration::cut_space() {
    while (_i_file < _file.size() && space(current_char())) {
        if (current_char() == '\n')
            ++_line;
        ++_i_file;
    }
}

void Server_configuration::comment() {
    while (_i_file < _file.size() && current_char() != '\n')
        ++_i_file;
}

void Server_configuration::token_quote() {
    size_t start = _i_file;
    size_t i = _i_file + 1;
    std::string value;
    char quote_char = _file[start];

    while (i < _file.size()) {
        if (quote_char == '"' && _file[i] == '\\') {
            if (i + 1 >= _file.size())
                throw std::invalid_argument(
                    msg_error("invalid escape at end of file", _line));

            if (_file[i + 1] == '"' || _file[i + 1] == '\\') {
                value += _file[i + 1];
                i += 2;
                continue;
            }

            else if (_file[i + 1] == 'n') {
                value += '\n';
                i += 2;
                continue;
            }

            else if (_file[i + 1] == 'r') {
                value += '\r';
                i += 2;
                continue;
            }

            else if (_file[i + 1] == 't') {
                value += '\t';
                i += 2;
                continue;
            }

            value += _file[i];
            ++i;
            continue;
        }

        if (_file[i] == '\n')
            ++_line;

        if (_file[i] == quote_char)
            break;

        value += _file[i];
        ++i;
    }

    if (i >= _file.size())
        throw std::invalid_argument(msg_error("unclosed quote", _line));

    if (i + 1 < _file.size() && !space(_file[i + 1]) &&
        !end_line(_file[i + 1]) && !directive(_file[i + 1]))
        throw std::invalid_argument(
            msg_error("invalid character after quote", _line));

    tokens_t token;
    token.status = WORD;
    token.token = value;
    token.line = _line;
    _tokens.push_back(token);

    _i_file = i + 1;
}

void Server_configuration::token_directive() {
    tokens_t token;

    if (current_char() == '{')
        token.status = OPEN_BLOCK;
    else if (current_char() == '}')
        token.status = CLOSE_BLOCK;
    else if (current_char() == ';')
        token.status = SEMICOLON;

    token.token = current_char();
    token.line = _line;
    _tokens.push_back(token);

    ++_i_file;
}

void Server_configuration::cut_word() {
    size_t start = _i_file;

    while (_i_file < _file.size() && !space(current_char()) &&
           !directive(current_char())) {
        if (quote(current_char()))
            throw std::invalid_argument(
                msg_error("quote after character in word", _line));
        ++_i_file;
    }

    tokens_t token;
    token.status = WORD;
    token.token = _file.substr(start, _i_file - start);
    token.line = _line;
    _tokens.push_back(token);
}
