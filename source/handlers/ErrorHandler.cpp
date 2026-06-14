# include "handlers/ErrorHandler.hpp"
# include "vserver/vserver.hpp"
# include "http/HTTP_Errors.hpp"
# include "tools/char_tools.hpp"

ErrorHandler::ErrorHandler(int code)
	:	_code(code)
{

}

ErrorHandler::~ErrorHandler()
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

static void	creation_error_file(request_context_t& context, resource_context_t& resource, int code)
{
	std::string	error_page;

	resource.mime_type = "text/html";

	error_page +=   "<!DOCTYPE html>\n"
					"<html lang=\"fr\">\n"
					"<head>\n"
					"  <meta charset=\"UTF-8\">\n"
					"  <title>Error " + char_tools::itostr(code) + "</title>\n"
					"  <link rel=\"stylesheet\" href=\"/styles/error.css\">\n"
					"</head>\n"

					"<body>\n"
					"  <div class=\"container\">\n"
					"    <div class=\"card\">\n"
					"      <h1 class=\"title\">Error " + char_tools::itostr(code) + "</h1>\n"
					"      <p class=\"message\">" + const_cast<char*>(http_error_reason(code)) + "</p>\n"
					"      <a class=\"button\" href=\"/\">Back to the home page</a>\n"
					"    </div>\n"
					"  </div>\n"
					"</body>\n"
					"</html>\n";

	std::string	name_page = context.root + "/tmp/" + get_time() + ".html";
	resource.file_tmp.fd = open(name_page.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
	if (resource.file_tmp.fd != -1)
	{
		size_t n_bits = 0;
		while (n_bits < error_page.size())
		{
			ssize_t bits_send = write(resource.file_tmp.fd, error_page.c_str() + n_bits, error_page.size() - n_bits);
			if (bits_send <= 0)
			{
				close(resource.file_tmp.fd);
				throw std::runtime_error("error write in file");
			}
			n_bits += static_cast<size_t>(bits_send);
			lseek(resource.file_tmp.fd, 0, SEEK_SET);
		}
		lseek(resource.file_tmp.fd, 0, SEEK_SET);

		GarbageCollector::get_instance().add_fd(resource.file_tmp.fd);	

		resource.file_tmp.size = error_page.size();
		resource.file_tmp.name_file = name_page;
	}
}

resource_context_t	ErrorHandler::execute(request_context_t& context)
{
	resource_context_t resource;

	if (context.errors.find(_code) != context.errors.end())
	{
		resource.file_read.fd = open(context.errors.find(_code)->second.c_str(), O_RDONLY);

		if (resource.file_read.fd != -1)
		{
			resource.mime_type = get_mime_type(context.errors.find(_code)->second);
			struct stat	status = {};
			stat(context.errors.find(_code)->second.c_str(), &status);
			resource.file_read.size = status.st_size;
			GarbageCollector::get_instance().add_fd(resource.file_read.fd);	
		}
	}
	else
		creation_error_file(context, resource, _code);


	resource.http.status = _code;
	resource.http.reason = const_cast<char*>(http_error_reason(_code));

	return resource;
}
