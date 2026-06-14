#ifndef WRAPPERS_HPP
# define WRAPPERS_HPP

# include <stdexcept>
# include <sys/socket.h>

# if defined(__linux__)

#  include <sys/epoll.h>

int	epoll_create_wrp();
int epoll_ctl_wrp(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait_wrp(int epfd, struct epoll_event *events, int maxevents, int timeout);

# elif defined(__APPLE__) && defined(__MACH__)

# endif

#endif
