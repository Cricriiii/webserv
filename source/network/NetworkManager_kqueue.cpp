# include "network/NetworkManager.hpp"
# include <csignal>
# include <iostream>
# include <stdexcept>
# include <sys/event.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <map>



static void	_kqueue_add_socket(int epfd, int socket)
{
	int	yes = 1;
	if (setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == -1)		throw std::runtime_error("setsockopt: SO_NOSIGPIPE");
	if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1)		throw std::runtime_error("fcntl: client");

	struct kevent	kev[2];
	EV_SET(&kev[0], socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	EV_SET(&kev[1], socket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
		
	if (kevent(epfd, kev, 2, NULL, 0, NULL) == -1)	throw std::runtime_error("kevent: add");
}

static void	_kqueue_mod_socket(int epfd, int socket, int event, int status)
{
	struct kevent	kev;
	EV_SET(&kev, socket, event, status, 0, 0, NULL);
	if (kevent(epfd, &kev, 1, NULL, 0, NULL) == -1)	throw std::runtime_error("kevent: modif");
}

void	NetworkManager::_init_event_polling()
{
	_event_fd = kqueue();
	GarbageCollector::get_instance().add_fd(_event_fd);

	if (_event_fd == -1)	throw std::runtime_error("kqueue");

	for (acceptor_list::iterator	it = _acceptors.begin(); it != _acceptors.end(); ++it)
	{
		int	sd = it->second->socketd();
		struct kevent	kev;
		EV_SET(&kev, sd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

		if (kevent(_event_fd, &kev, 1, NULL, 0, NULL) == -1)	throw std::runtime_error("kevent: signal");
	}
}

void	NetworkManager::listen()
{
	std::cout << "Listening for new connections..." << std::endl;
	int	nb_events = kevent(_event_fd, NULL, 0, &_events[0], 256, NULL);
	if (nb_events == -1)	throw std::runtime_error("kevent: run");

	for (int i = 0; i < nb_events; ++i)
	{
		int	sd = _events[i].ident;
		if (_acceptors.find(sd) != _acceptors.end())
		{
			while (1)
			{
				int	accepted = _acceptors[sd]->accept();
				if (accepted == -1) break;
				
				_connections.insert(std::make_pair(accepted, NetConnection(accepted, _http_conf, _obuffer.capacity)));

				_kqueue_add_socket(_event_fd, accepted);
				_connections.find(accepted)->second.set_state(S_CONN_READ);
			}
		}
		else
		{
			http_conn_list::iterator	current_connection = _connections.find(sd);
			if (current_connection != _connections.end()) {
				poll_state_t	poll_state;

				if (_events[i].filter == EVFILT_READ && _events[i].filter == EVFILT_WRITE)
				{
					poll_state = S_POLL_IN_OUT;
				}
				else if (_events[i].filter == EVFILT_READ)
				{
					poll_state = S_POLL_IN;
				}
				else if (_events[i].filter == EVFILT_WRITE)
				{
					poll_state = S_POLL_OUT;
				}
				else if (_events[i].flags & (EV_EOF | EV_ERROR))
				{
					poll_state = S_POLL_ERR;
				}
				else
				{
					poll_state = S_POLL_NONE;
				}
				current_connection->second.set_poll_state(poll_state);
				 if (!current_connection->second.ready()) {
					_ready_lst.push_back(sd);
					current_connection->second.set_readiness(true);
				}
			}
		}
	}
}

void	NetworkManager::_mod_client_interest_set(int socketd, poll_op_t new_state)
{
	if (new_state == (SCAN_EVENT_IN | SCAN_EVENT_OUT)) {
		_kqueue_mod_socket(_event_fd, socketd, EVFILT_READ, EV_ENABLE);
		_kqueue_mod_socket(_event_fd, socketd, EVFILT_WRITE, EV_ENABLE);
	}
	else if (new_state & SCAN_EVENT_IN) {
		_kqueue_mod_socket(_event_fd, socketd, EVFILT_READ, EV_ENABLE);
		_kqueue_mod_socket(_event_fd, socketd, EVFILT_WRITE, EV_DISABLE);
	}
	else if (new_state & SCAN_EVENT_OUT) {
		_kqueue_mod_socket(_event_fd, socketd, EVFILT_WRITE, EV_ENABLE);
		_kqueue_mod_socket(_event_fd, socketd, EVFILT_READ, EV_DISABLE);
	}
}