# include "vserver/vserver_creator.hpp"
# include "vserver/vserver.hpp"
# include <stdexcept>



VServerCreator::VServerCreator()		{}

VServerCreator::~VServerCreator()		{}



std::vector<VServer>	VServerCreator::create_vservers(const std::vector<vserver_t>& confs)
{
	std::vector<VServer>	vservers;

	for (size_t i = 0; i < confs.size(); i++)
	{
		if (confs[i].name.empty())						throw std::invalid_argument("server: empty name in server configuration");

		if (confs[i].port < 0 || confs[i].port > 65535)	throw std::invalid_argument("server: invalid port in server configuration");

		if (confs[i].index.empty())						throw std::invalid_argument("server: empty index in server configuration");

		if (confs[i].root.empty())						throw std::invalid_argument("server: empty root in server configuration");

		vservers.push_back(VServer(confs[i]));
	}
	return (vservers);
}
