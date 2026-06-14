/* ==============[preprocessor]========================================= */

#include "network/NetworkManager.hpp"
#include "wrappers.hpp"


/* ==============[common]=============================================== */

static void	_epoll_add_socket(int epfd, int socket, int event)
{
	struct epoll_event	ev = {};
	ev.data.fd = socket;
	ev.events = event;
	epoll_ctl_wrp(epfd, EPOLL_CTL_ADD, socket, &ev);
}

static void	_epoll_mod_socket(int epfd, int socket, int event)
{
	struct epoll_event	ev = {};
	ev.data.fd = socket;
	ev.events = event;
	epoll_ctl_wrp(epfd, EPOLL_CTL_MOD, socket, &ev);
}


/* ==============[NetworkManager._init]======================================== */

void	NetworkManager::_init_event_polling()
{	
	_event_fd = epoll_create_wrp();
	GarbageCollector::get_instance().add_fd(_event_fd);

	for (acceptor_list::iterator	it = _acceptors.begin(); it != _acceptors.end(); ++it)
	{	
		int	sd = it->second->socketd();
		if (sd != -1) {
			_epoll_add_socket(_event_fd, sd, EPOLLIN);
		}
	}
}


/* ==============[NetworkManager.listen]============================================= */

void	NetworkManager::listen()
{
	int	_n_events = epoll_wait_wrp(_event_fd, _events, MAX_EVENTS, -1);

	for (int i = 0; i < _n_events; ++i) {
		
		int	sd = _events[i].data.fd;
		if (_acceptors.find(sd) != _acceptors.end()) {
			
			while (1) {
				int	accepted = _acceptors[sd]->accept();

				if (accepted == -1) {
					break;
				}
				
				_connections.insert(std::make_pair(accepted, NetConnection(accepted, _http_conf, _obuffer.capacity)));

				_epoll_add_socket(_event_fd, accepted, EPOLLIN);
				_connections.find(accepted)->second.set_state(S_CONN_READ);
			}
		}
		else {

			http_conn_list::iterator	current_connection = _connections.find(sd);
			if (current_connection != _connections.end()) {

				poll_state_t	poll_state;

				if ((_events[i].events & EPOLLHUP) || (_events[i].events & EPOLLERR)) {
					poll_state = S_POLL_ERR;
				}
				else if ((_events[i].events & EPOLLIN) && (_events[i].events & EPOLLOUT)) {
					poll_state = S_POLL_IN_OUT;
				}
				else if (_events[i].events & EPOLLOUT) {
					poll_state = S_POLL_OUT;
				}
				else if (_events[i].events & EPOLLIN) {
					poll_state = S_POLL_IN;
				}
				else {
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


/* ==================[NetworkManager._mod_client_interest_set]=============================================== */

void	NetworkManager::_mod_client_interest_set(int socketd, poll_op_t new_state)
{
	if (new_state == (SCAN_EVENT_IN | SCAN_EVENT_OUT)) {
		_epoll_mod_socket(_event_fd, socketd, EPOLLIN | EPOLLOUT);
	}
	else if (new_state & SCAN_EVENT_IN) {
		_epoll_mod_socket(_event_fd, socketd, EPOLLIN);
	}
	else if (new_state & SCAN_EVENT_OUT) {
		_epoll_mod_socket(_event_fd, socketd, EPOLLOUT);
	}
}


