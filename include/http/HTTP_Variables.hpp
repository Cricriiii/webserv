#ifndef HTTP_VARIABLES_HPP
# define HTTP_VARIABLES_HPP

# include <cstddef>

typedef struct response {
	size_t	cur;
	bool	done;
	int		fd;

	response(): cur(0), done(false), fd(-1) {}
	response(size_t _cur, bool _done, int _fd): cur(_cur), done(_done), fd(_fd) {}
} response_status_t;

#endif
