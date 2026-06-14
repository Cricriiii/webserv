#ifndef STATICFILEHANDLER_HPP
# define STATICFILEHANDLER_HPP

# include "handlers/IHandler.hpp"
# include "handlers/HandlerValidator.hpp"
# include "handlers/methods.hpp"


class StaticFileHandler: public IHandler
{
	private:
	resource_context_t	get_method(request_context_t& context);
	resource_context_t	post_method(request_context_t& context);
	resource_context_t	delete_method(request_context_t& context);
	resource_context_t	put_method(request_context_t& context);
	
	public:
	StaticFileHandler();
	~StaticFileHandler();
	
	resource_context_t	execute(request_context_t& request);
	
};

typedef resource_context_t (StaticFileHandler::*ptr_fct)(request_context_t&);

#endif
