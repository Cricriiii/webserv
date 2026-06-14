# include "handlers/HandlerValidator.hpp"
# include "handlers/ErrorHandler.hpp"
# include "handlers/CGIHandler.hpp"
# include "handlers/AutoIndexHandler.hpp"
# include "handlers/StaticFileHandler.hpp"
# include "http/HTTP_Errors.hpp"
# include <unistd.h>



HandlerValidator::HandlerValidator()	{}

HandlerValidator::~HandlerValidator()	{}



static IHandler*	verif_url(request_context_t& context)
{
	if (context.request->start_line.URL.find("/..") != std::string::npos)
		return new ErrorHandler(E_HTTP_UNAUTHORIZED);
	if (context.directory)
	{
		if (access(context.resolved_path.c_str(), F_OK) == -1)
			return new ErrorHandler(E_HTTP_NOT_FOUND);

		for (size_t i = 0; i < context.index.size(); ++i)
		{
			std::string complet_path = context.resolved_path + context.index[i];
			if (access(complet_path.c_str(), F_OK) == 0)
			{
				if (access(complet_path.c_str(), R_OK) == -1 && !context.autoindex)
					return new ErrorHandler(E_HTTP_FORBIDDEN);
				else
				{
					context.resolved_path = complet_path;
					return NULL;
				}
			}
		}
		if (context.request->start_line.method == "POST")
			return NULL;
		else if (context.autoindex)
			return new AutoIndexHandler();
		else
		 	return new ErrorHandler(E_HTTP_FORBIDDEN);
	}
	else
	{
		if (context.request->start_line.method == "PUT")
			return NULL;

		struct stat	status = {};
		if (stat(context.resolved_path.c_str(), &status) == -1)
			return new ErrorHandler(E_HTTP_NOT_FOUND);
		if (S_ISDIR(status.st_mode))
			return new ErrorHandler(E_HTTP_FORBIDDEN);

		if (access(context.resolved_path.c_str(), F_OK) == 0)
		{
			if (access(context.resolved_path.c_str(), R_OK) == -1)
				return new ErrorHandler(E_HTTP_FORBIDDEN);
			else
				return NULL;
		}
		else
			return new ErrorHandler(E_HTTP_NOT_FOUND);
	}
}

static IHandler*	verif_method(request_context_t& context)
{
	for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i)
	{
		if (context.request->start_line.method == methods[i])
		{
			if (!context.limit_except.empty() && context.limit_except.find(context.request->start_line.method) != context.limit_except.end())
				return new ErrorHandler(E_HTTP_METHOD_NOT_ALLOWED);
			else
				return NULL;
		}
	}
	return new ErrorHandler(E_HTTP_NOT_IMPLEMENTED);
}

static bool	verif_cgi_extension(const std::string& path, const std::string& cgi_extension)
{
	if (path.size() < cgi_extension.size())
		return false;

	return (path.compare(path.size() - cgi_extension.size(), cgi_extension.size(), cgi_extension) == 0);
}

static IHandler*	verif_cgi(request_context_t& context)
{
	if (context.cgi)
	{
		if (context.cgi_extension.empty() == false && context.cgi_path.empty() == false)
		{
			if (access(context.resolved_path.c_str(), F_OK | R_OK) == -1)
				return new ErrorHandler(E_HTTP_INTERNAL_SERVER_ERROR);
			else
				return new CGIHandler();
		}
		else
		{
			if (verif_cgi_extension(context.resolved_path, context.cgi_extension) == false)
				return new ErrorHandler(E_HTTP_FORBIDDEN);

			if (access(context.resolved_path.c_str(), F_OK) == -1)
				return new ErrorHandler(E_HTTP_NOT_FOUND);
			else if (access(context.resolved_path.c_str(), X_OK) == -1)
				return new ErrorHandler(E_HTTP_FORBIDDEN);
			else if (access(context.cgi_path.c_str(), F_OK | X_OK) == -1)
				return new ErrorHandler(E_HTTP_INTERNAL_SERVER_ERROR);
			else
				return new CGIHandler();
		}
	}
	return NULL;
}

static bool	verif_handlers(request_context_t& context)
{
	std::map<std::string, std::string>::const_iterator it;

	it = context.request->headers.find("transfer-encoding");

	if (it != context.request->headers.end())
	{
		if (it->second.compare("chunked") != 0)
			return false;
	}
	return true;
}

IHandler*	HandlerValidator::validator(request_context_t& context)
{
	if (HTTP_IS_ERR(context.request->status))
	{
		return new ErrorHandler(context.request->status);
	}

	if (!verif_handlers(context))
	{
		return new ErrorHandler(E_HTTP_NOT_IMPLEMENTED);
	}

	{
	IHandler* handler = verif_url(context);
	if (handler)
		return handler;
	}

	if (context.cgi)
	{
		IHandler* handler = verif_cgi(context);
		if (handler)
			return handler;
	}

	{
	IHandler* handler = verif_method(context);
	if (handler)
		return handler;
	}

	return new StaticFileHandler();
}
