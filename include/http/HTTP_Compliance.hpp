#ifndef HTTP_COMPLIANCE_HPP
#define HTTP_COMPLIANCE_HPP

#include <netinet/in.h>

#include <limits>
#include <string>

#include "Constants.hpp"
#include "http/HTTP_ASCII.hpp"
#include "http/HTTP_Errors.hpp"
#include "tools/StringHandler.hpp"

typedef enum hostname_type {
    NONE_HOSTNAME,
    IPV4_HOSTNAME,
    IPV6_HOSTNAME,
    DNS_HOSTNAME
} hostname_type;

struct compliance_info {
    int port;
    std::string domain;
    http_status_t status;
    hostname_type name_type;

    compliance_info()
        : port(-1), domain(), status(E_HTTP_OK), name_type(NONE_HOSTNAME) {
    }
};

namespace HTTP_Compliance {
static compliance_info _domain_name_compliance(const std::string& hostname);
static compliance_info _ipv4_host_compliance(const std::string& hostname);
static compliance_info _ipv6_host_compliance(const std::string& hostname);
}  // namespace HTTP_Compliance

#include "http/HTTP_Compliance.tpp"

#endif
