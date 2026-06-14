#include "app.hpp"
#include "network/NetworkManager.hpp"
#include "conf.hpp"
#include <exception>
#include <iostream>
#include "vserver/vserver_creator.hpp"

#include <signal.h>

volatile sig_atomic_t	run_server = true;

void	shutdown_server(int signal)
{
	(void) signal;
	run_server = false;
}

static void	msg_error(const std::string &s1)
{
	if (s1 != "Error: epoll_wait()")
		std::cerr << RED BOLD << "Error: " << RESET RED << s1 << RESET << std::endl;
}


static void	real_main(int argc, char **argv)
{
	if (argc != 2)
		throw std::invalid_argument("usage: ./webserv <path_to_conf_file>");

	Server_configuration	config(argv[1]);

	std::vector<VServer>	vservers = VServerCreator::create_vservers(config.get_list_vservers());
	
	NetworkManager	nm(vservers, config.get_list_port(), config.get_conf_http());
		
	while (run_server) {
		nm.listen();
		nm.execute();
	}
}


int	main(int argc, char **argv)
{
	signal(SIGINT, &shutdown_server);
	signal(EINTR, &shutdown_server);
	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));

	try {
		real_main(argc, argv);
	}
	catch(std::exception& e) {
		if (errno != EINTR) {
			msg_error(e.what());
		}
		return 1;
	}
	return 0;
}
