# include "conf.hpp"
# include <iostream>



Server_configuration::Server_configuration(const char *path_file) : _line(1), _file(), _i_file(0), _tokens(), _i_token(0), _vservers(), _ports(), _conf_http()
{
	open_read_file(path_file);
	lexer();
	parsing();
	fill_ports();
}

Server_configuration::~Server_configuration()		{}



void	Server_configuration::fill_ports()
{
	vservers_it_t	it = _vservers.begin();
	vservers_it_t	ite = _vservers.end();

	for (; it != ite; ++it)
		_ports.insert(it->port);
}

const std::vector<vserver_t>&	Server_configuration::get_list_vservers()	const
{
	return _vservers;
}

const std::set<int>&	Server_configuration::get_list_port()	const
{
	return _ports;
}

const conf_http_t&	Server_configuration::get_conf_http()	const
{
	return _conf_http;
}

void	Server_configuration::print_conf() const
{
	std::cout << "\n\n-------------------- CONFIGURATION --------------------\n" << std::endl;

	std::cout << "--------------------- SERVERS --------------------\n" << std::endl;

	for (size_t i = 0; i < _vservers.size(); i++)
	{
		std::cout << "\nserver " << i + 1 << ":" << std::endl;
		for (std::set<std::string>::const_iterator it = _vservers[i].name.begin(); it != _vservers[i].name.end(); ++it)
			std::cout << "server_name: " << *it << std::endl;
		std::cout << "port: " << _vservers[i].port << std::endl;
		std::cout << "root: " << _vservers[i].root << std::endl;
		std::cout << "max_body_size: " << _vservers[i].max_body_size << std::endl;

		for (size_t j = 0; j < _vservers[i].index.size(); j++)
			std::cout << "index: " << _vservers[i].index[j] << std::endl;

		for (std::map<int, std::string>::const_iterator it = _vservers[i].errors.begin(); it != _vservers[i].errors.end(); ++it)
			std::cout << "error_page: " << it->first << " | path: " << it->second << std::endl;

		for (size_t k = 0; k < _vservers[i].location.size(); k++)
		{
			std::cout << std::endl;
			std::cout << "\tlocation path: " << _vservers[i].location[k].path << std::endl;
			std::cout << "\tautoindex: " << (_vservers[i].location[k].autoindex ? "on" : "off") << std::endl;
			std::cout << "\troot: " << _vservers[i].location[k].root << std::endl;

			for (size_t l = 0; l < _vservers[i].location[k].index.size(); l++)
				std::cout << "\tindex: " << _vservers[i].location[k].index[l] << std::endl;

			for (std::set<std::string>::const_iterator it = _vservers[i].location[k].limit_except.begin(); it != _vservers[i].location[k].limit_except.end(); ++it)
				std::cout << "\tlimit_except: " << *it << std::endl;
		}
	}

	std::cout << "\n--------------------- PORTS USED --------------------\n" << std::endl;

	for (std::set<int>::const_iterator it = _ports.begin(); it != _ports.end(); ++it)
		std::cout << "\nport used: " << *it << std::endl;

	std::cout << "\n--------------------- HTTP CONFIGURATION --------------------\n" << std::endl;

	std::cout << "\nhttp_max_body_siz: " << _conf_http.http_max_body_siz
			  << "\nrecv_segment_siz: " << _conf_http.recv_segment_siz
			  << "\nsend_segment_siz: " << _conf_http.send_segment_siz
			  << "\nhttp_max_method_siz: " << _conf_http.http_max_method_siz
			  << "\nhttp_max_uri_siz: " << _conf_http.http_max_uri_siz
			  << "\nhttp_max_header_key_siz: " << _conf_http.http_max_header_key_siz
			  << "\nhttp_max_header_val_siz: " << _conf_http.http_max_header_val_siz
			  << std::endl;

	std::cout << "\n-------------------- END CONFIGURATION --------------------\n" << std::endl;
}
