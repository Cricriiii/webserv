#ifndef CGIHandler_HPP
# define CGIHandler_HPP

# include "handlers/IHandler.hpp"

static const char*	headers_blacklist[] =
{
		"authorization",
		"content-type",
		"content-length",
		"connection",
		"keep-alive",
		"proxy-connection",
		"transfer-encoding",
		"upgrade",
		"te",
		"trailer"
};

class CGIHandler: public IHandler
{
public:
	CGIHandler();
	~CGIHandler();

	resource_context_t	execute(request_context_t& context);

};

#endif
