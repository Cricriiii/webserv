# include "handlers/StaticFileHandler.hpp"
# include <sys/time.h>
# include "tools/StringHandler.hpp"
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>



StaticFileHandler::StaticFileHandler()
{

}

StaticFileHandler::~StaticFileHandler()
{

}

static std::string	get_time()
{
    time_t		now = time(NULL);
    struct tm	*t = localtime(&now);
	std::string time_str;
	static int	old_sec = 0;
	static unsigned long long	n_call = 0;
	t->tm_year += 1900;
	t->tm_mon += 1;

	StringHandler	str_hdlr(time_str);
	str_hdlr += t->tm_year;
	if (t->tm_mon < 10)
		str_hdlr += "0";
	str_hdlr += t->tm_mon;
	if (t->tm_mday < 10)
		str_hdlr += "0";
	str_hdlr += t->tm_mday;
	str_hdlr += "-";
	if (t->tm_hour < 10)
		str_hdlr += "0";
	str_hdlr += t->tm_hour;
	str_hdlr += "-";
	if (t->tm_min < 10)
		str_hdlr += "0";
	str_hdlr += t->tm_min;
	str_hdlr += "-";
	if (t->tm_sec < 10)
		str_hdlr += "0";
	str_hdlr += t->tm_sec;

	if (old_sec != t->tm_sec)
	{
		n_call = 0;
		old_sec = t->tm_sec;
	}

	str_hdlr += "_";
	n_call++;
	if (n_call < 10)
		str_hdlr += "0";
	str_hdlr += n_call;

    return (time_str);
}



resource_context_t	StaticFileHandler::execute(request_context_t& context)
{
	static ptr_fct	ft_methods[5] = {&StaticFileHandler::get_method, &StaticFileHandler::post_method, &StaticFileHandler::delete_method, &StaticFileHandler::put_method};

	size_t i = 0;
	for (; i < sizeof(methods) / sizeof(char*); i++)
	{
		if (context.request->start_line.method == methods[i])
			break;
	}
	return (this->*ft_methods[i])(context);
}



resource_context_t	StaticFileHandler::get_method(request_context_t& context)
{
	resource_context_t resource;

	resource.mime_type = get_mime_type(context.resolved_path);
	resource.file_read.fd = open(context.resolved_path.c_str(), O_RDONLY);
	GarbageCollector::get_instance().add_fd(resource.file_read.fd);

	struct stat	status = {};
	if (stat(context.resolved_path.c_str(), &status) == -1)
		throw std::runtime_error("Failed to stat file");
	resource.file_read.size = status.st_size;

	if (S_ISDIR(status.st_mode))
	{
		throw std::runtime_error("Failed to open file: is a directory");
	}

	resource.http.status = E_HTTP_OK;
	resource.http.reason = const_cast<char*>(http_error_reason(E_HTTP_OK));

	return resource;
}

resource_context_t	StaticFileHandler::post_method(request_context_t& context)
{
	resource_context_t	resource;

	HTTP_Request::header_it_t it = context.request->headers.find("content-type");
	std::string extension = ".tmp";

	if (it != context.request->headers.end())
	{
		std::string found_ext = get_extension(it->second);
		if (!found_ext.empty()) {
			extension = found_ext;
		}
	}

	std::string filename = context.resolved_path + get_time() + extension;
	resource.file_write.fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_EXCL, 0644);

	if (resource.file_write.fd == -1)
		throw std::runtime_error("Failed to create file in upload path");

	GarbageCollector::get_instance().add_fd(resource.file_write.fd);

	resource.file_write.size = context.request->entity_body.size;

	resource.http.status = E_HTTP_OK;
	resource.http.reason = const_cast<char*>(http_error_reason(E_HTTP_OK));

	return resource;
}

resource_context_t	StaticFileHandler::delete_method(request_context_t& context)
{
	resource_context_t resource;

	struct stat	status = {};
	if (stat(context.resolved_path.c_str(), &status) == -1)
		throw std::runtime_error("Failed to stat file");
	if (S_ISDIR(status.st_mode))
	{
		throw std::runtime_error("Failed to open file: is a directory");
	}

	if (unlink(context.resolved_path.c_str()) == -1)
		throw std::runtime_error("Failed to delete file");

	resource.http.status = E_HTTP_OK;
	resource.http.reason = const_cast<char*>(http_error_reason(E_HTTP_OK));

	return resource;
}

resource_context_t	StaticFileHandler::put_method(request_context_t& context)
{
	resource_context_t resource;

	resource.file_write.fd = open(context.resolved_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);

	if (resource.file_write.fd == -1)
		throw std::runtime_error("Failed to create file in upload path");

	GarbageCollector::get_instance().add_fd(resource.file_write.fd);

	resource.file_write.size = context.request->entity_body.size;

	resource.http.status = E_HTTP_OK;
	resource.http.reason = const_cast<char*>(http_error_reason(E_HTTP_OK));

	return resource;
}
