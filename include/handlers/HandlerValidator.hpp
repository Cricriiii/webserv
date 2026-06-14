#ifndef HANDLERVALIDATOR_HPP
# define HANDLERVALIDATOR_HPP

# include "vserver/vserver.hpp"
# include "handlers/IHandler.hpp"
# include "handlers/methods.hpp"



class HandlerValidator
{
	private:
		HandlerValidator();
		~HandlerValidator();

	public:
		static IHandler*	validator(request_context_t& context);
};

#endif
