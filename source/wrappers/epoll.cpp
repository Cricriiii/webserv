#include "wrappers.hpp"

int	epoll_create_wrp()
{
	int	epfd = epoll_create(1);
	
	if (epfd == -1) {
		throw std::runtime_error("Error: epoll_create()");
	}
	else {
		return epfd;
	}
}

int epoll_ctl_wrp(int epfd, int op, int fd, struct epoll_event *event)
{
	int	ctl = epoll_ctl(epfd, op, fd, event);

	if (ctl == -1) {
		throw std::runtime_error("Error: epoll_ctl()");
	}
	else {
		return ctl;
	}	
}

int epoll_wait_wrp(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int n_events = epoll_wait(epfd, events, maxevents, timeout);

	if (n_events == -1) {
		throw std::runtime_error("Error: epoll_wait()");
	}
	else {
		return n_events;
	}
}
