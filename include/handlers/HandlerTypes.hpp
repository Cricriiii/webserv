#ifndef HANDLERTYPES_HPP
# define HANDLERTYPES_HPP

# include <string>

struct file_context {
	int			fd;
	size_t		size;
	std::string	name_file;
	
	file_context(): fd(-1), size(0), name_file() {}
};

struct http_context {
	int		status;
	char*	reason;
	
	http_context(): status(), reason(NULL) {}
};

struct cgi_info {
	pid_t				process;
	long long			start_cgi;

	cgi_info(): process(-1), start_cgi(0) {}
};

typedef struct resource_context
{
	std::string			mime_type;

	struct file_context	file_write;
	struct file_context	file_read;
	struct file_context	file_tmp;
	struct file_context	file_cgi_write;
	struct file_context	file_cgi_read;
	struct cgi_info		cgi;
	struct http_context	http;
	
	resource_context(): mime_type(), file_write(), file_read(), file_tmp(), file_cgi_write(), file_cgi_read(), cgi(), http() {}
}	resource_context_t;

#endif