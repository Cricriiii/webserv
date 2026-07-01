#include "http/HTTP_Response.hpp"

/* [constructors */

HTTP_Response::HTTP_Response()
    : start_line(), headers(), entity_body(), raw_bytes_sent(0), size(0) {
}

HTTP_Response::HTTP_Response(const HTTP_Response& other)
    : start_line(other.start_line),
      headers(other.headers),
      entity_body(other.entity_body),
      raw_bytes_sent(other.raw_bytes_sent),
      size(other.size) {
}

HTTP_Response& HTTP_Response::operator=(const HTTP_Response& other) {
    if (this != &other) {
        start_line = other.start_line;
        headers = other.headers;
        entity_body = other.entity_body;
        raw_bytes_sent = other.raw_bytes_sent;
        size = other.size;
    }
    return *this;
}

HTTP_Response::~HTTP_Response() {
}

/* [clear] */

void HTTP_Response::clear() {
    *this = HTTP_Response();
}

/* [overloads */

std::ostream& operator<<(std::ostream& os, const HTTP_Response& response) {
    os << "\r\n"
       << response.start_line.version.name << '/'
       << response.start_line.version.major << '.'
       << response.start_line.version.minor << ' '
       << response.start_line.status.code << ' '
       << response.start_line.status.reason << "\r\n";

    for (HTTP_Response::header_cit_t cit = response.headers.begin();
         cit != response.headers.end(); ++cit) {
        os << cit->first << ':' << cit->second << "\r\n";
    }

    os << response.entity_body.data << "\r\n";

    return os;
}

StringHandler& operator<<(StringHandler& sb, const HTTP_Response& response) {
    sb << "\r\n"
       << response.start_line.version.name << '/'
       << response.start_line.version.major << '.'
       << response.start_line.version.minor << ' '
       << response.start_line.status.code << ' '
       << response.start_line.status.reason << "\r\n";

    for (HTTP_Response::header_cit_t cit = response.headers.begin();
         cit != response.headers.end(); ++cit) {
        sb << cit->first << ':' << cit->second << "\r\n";
    }

    sb << "\r\n" << response.entity_body.data << "\r\n";

    return sb;
}
