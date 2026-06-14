#include "network/Router.hpp"
#include <limits>
#include "tools/StringHandler.hpp"
#include "Constants.hpp"
#include "http/HTTP_ASCII.hpp"
#include "http/HTTP_Compliance.hpp"

Router::Router(std::vector<VServer>& vs)
	:	_vservers(vs)
{
}


Router::~Router()
{
}


static const std::string&	_get_host_str(const HTTP_Request::header_t& headers)
{
	HTTP_Request::header_cit_t	host_head = headers.find("host");

	if (host_head == headers.end()) {
		throw std::runtime_error("Router: Should never reach router.route() without host header");
	}

	return host_head->second;
}


struct compliance_info Router::_host_validity(const std::string& hostname)
{
	if (hostname.empty()) {
		struct compliance_info	c;
		c.status = E_HTTP_BAD_REQUEST;
		return c;
	}

	if (hostname[0] == '[') {
		return HTTP_Compliance::_ipv6_host_compliance(hostname);
	}
	else if (IS_DIGIT(hostname[0])) {
		return HTTP_Compliance::_ipv4_host_compliance(hostname);
	}
	else {
		return HTTP_Compliance::_domain_name_compliance(hostname);
	}
}


int Router::_getsock_proto_info(int socketd, int* port, void* ip_bin, int* proto_family) const
{
	struct sockaddr_storage	addr_store = {};
	socklen_t				len = sizeof(addr_store);

	if (getsockname(socketd, reinterpret_cast<struct sockaddr*>(&addr_store), &len) == -1) {
		return -1;
	}

	*proto_family = addr_store.ss_family;

	if (addr_store.ss_family == AF_INET) {
		struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(&addr_store);
		*port = ntohs(sin->sin_port);
		
		if (ip_bin)
			*reinterpret_cast<struct in_addr*>(ip_bin) = sin->sin_addr;

		return 0;
	}
	else if (addr_store.ss_family == AF_INET6) {
		struct sockaddr_in6* sin6 = reinterpret_cast<struct sockaddr_in6*>(&addr_store);
		*port = ntohs(sin6->sin6_port);

		if (ip_bin)
			*reinterpret_cast<struct in6_addr*>(ip_bin) = sin6->sin6_addr;

		return 0;
	}
	else {
		return -1;
	}
}


request_context_t	Router::route(HTTP_Request& request)
{
	int				proto_family = 0;
	int				port = -1;
	unsigned char*	buffer_ip[INET6_ADDRSTRLEN] = {0};

	if (HTTP_IS_ERR(request.status)) {
		return _vservers.begin()->execute(request);
	}

	if (_getsock_proto_info(request.socketd, &port, buffer_ip, &proto_family) == -1) {
		request.status = E_HTTP_INTERNAL_SERVER_ERROR;
		return _vservers.begin()->execute(request);
	}

	const std::string&	full_host_str = _get_host_str(request.headers);
	struct compliance_info	compliance = _host_validity(full_host_str);

	if (!HTTP_IS_ERR(compliance.status)) {
		if (compliance.port != -1 && port != compliance.port) {
			request.status = E_HTTP_BAD_REQUEST;
			return _vservers.begin()->execute(request);
		}
		else {
			for (vserv_it_t it = _vservers.begin(); it != _vservers.end(); ++it) {
				if (it->is_it_me(compliance.domain, port)) {
					return it->execute(request);
				}
			}
			for (vserv_it_t it = _vservers.begin(); it != _vservers.end(); ++it) {
				if (it->is_port(port)) {
					return it->execute(request);
				}
			}
			request.status = E_HTTP_BAD_REQUEST;	
			return _vservers.begin()->execute(request);
		}
	}
	else {
		request.status = compliance.status;
		return _vservers.begin()->execute(request);
	}
}
