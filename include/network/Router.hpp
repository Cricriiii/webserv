#ifndef ROUTER_HPP
# define ROUTER_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include "vserver/vserver.hpp"
# include "http/HTTP_Request.hpp"
# include "http/HTTP_Response.hpp"
# include "HTTP_Errors.hpp"

# include "vserver/vserver.hpp"

class Router
{
public:
	typedef std::vector<VServer>			vserver_list;
	typedef vserver_list::iterator			vserv_it_t;
	typedef vserver_list::const_iterator	vserv_cit_t;

	Router(std::vector<VServer>& vs);
	~Router();

	request_context_t	route(HTTP_Request& request);
	
private:
	int						_getsock_proto_info(int socketd, int* port, void* ip_bin, int* proto_family) const;
	struct compliance_info	_host_validity(const std::string& hostname);

	vserver_list&	_vservers;

};

#endif
