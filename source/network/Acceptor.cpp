#include "network/Acceptor.hpp"
#include <sstream>
#include "Constants.hpp"


/* ==================[CONSTRUCTORS, DESTRUCTORS]================================= */

Acceptor::Acceptor(int port, protocol_type protocol)
	:	_socketd(-1),
		_port(port)
{
	try {
		if (port < PORT_MIN_VALUE || port > PORT_MAX_VALUE)
			throw std::invalid_argument("Acceptor: invalid port in server configuration");

		struct sockaddr_storage	addr = {};
		int	yes = 1;

		switch (protocol) {

			case PROTO_IPV4:

				if ( (_socketd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
					throw std::runtime_error("Acceptor: socket creation failed");

				GarbageCollector::get_instance().add_fd(_socketd);

				if (fcntl(_socketd, F_SETFL, O_NONBLOCK) == -1) {
					throw std::runtime_error("fcntl: O_NONBLOCK");
				}

				((struct sockaddr_in*)&addr)->sin_family = AF_INET;
				((struct sockaddr_in*)&addr)->sin_port = htons(_port);
				((struct sockaddr_in*)&addr)->sin_addr.s_addr = htonl(INADDR_ANY);

				if (setsockopt(_socketd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
					throw std::runtime_error("setsockopt: SO_REUSEADDR IPV4");
				}

				if (bind(_socketd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
					throw std::runtime_error("bind: IPV4");
				}

				if (listen(_socketd, SOMAXCONN) == -1) {
					throw std::runtime_error("listen: IPV4");
				}

				break;


			case PROTO_IPV6:

				if ( (_socketd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
					throw std::runtime_error("Acceptor: socket creation failed");

				GarbageCollector::get_instance().add_fd(_socketd);

				if (fcntl(_socketd, F_SETFL, O_NONBLOCK) == -1) {
					throw std::runtime_error("fcntl: O_NONBLOCK");
				}

				((struct sockaddr_in6*)&addr)->sin6_family = AF_INET6;
				((struct sockaddr_in6*)&addr)->sin6_port = htons(_port);
				((struct sockaddr_in6*)&addr)->sin6_addr = in6addr_any;

				if (setsockopt(_socketd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
					throw std::runtime_error("setsockopt: SO_REUSEADDR IPV6");
				}

				if (setsockopt(_socketd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1) {
					throw std::runtime_error("setsockopt: IPV6_V6ONLY");
				}
				
				if (bind(_socketd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in6)) == -1) {
					throw std::runtime_error("bind: IPV6");
				}

				if (listen(_socketd, SOMAXCONN) == -1) {
					throw std::runtime_error("listen: IPV6");
				}

				break;			


			default:
				throw std::runtime_error("Acceptor: unknown protocol");

				break;
		}
	}
	catch (std::exception &e) {
		close();
		throw;
	}
}


Acceptor::~Acceptor()
{
	close();
}





/* ==================[ACCEPT]=========================================================================================================== */

int	Acceptor::accept()
{
	int	descriptor = ::accept(_socketd, NULL, NULL);
	if (descriptor != -1 && fcntl(descriptor, F_SETFL, O_NONBLOCK) == -1) {
		throw std::runtime_error("fcntl: O_NONBLOCK");
	}
	GarbageCollector::get_instance().add_fd(descriptor);
	return descriptor;
}





/* ==================[SOCKETD]========================================================================================================== */

int		Acceptor::socketd() const
{
	return _socketd;
}





/* ==================[CLOSE]============================================================================================================ */

void	Acceptor::close()
{
	if (_socketd != -1) ::close(_socketd);
}