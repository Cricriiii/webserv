#ifndef IHANDLER_HPP
# define IHANDLER_HPP

# include "GarbageCollector.hpp"

# include "vserver/vserver.hpp"
# include "http/HTTP_Response.hpp"
# include "http/HTTP_Errors.hpp"
# include "handlers/HandlerTypes.hpp"
# include "http/HTTP_Errors.hpp"
# include <fcntl.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "handlers/mime_types.hpp"



class IHandler
{
	public:
		virtual ~IHandler() {};

		virtual resource_context_t	execute(request_context_t& context) = 0;
};

#endif
