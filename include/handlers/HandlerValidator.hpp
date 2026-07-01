#ifndef HANDLERVALIDATOR_HPP
#define HANDLERVALIDATOR_HPP

#include "handlers/IHandler.hpp"
#include "handlers/methods.hpp"
#include "vserver/vserver.hpp"

class HandlerValidator {
private:
    HandlerValidator();
    ~HandlerValidator();

public:
    static IHandler* validator(request_context_t& context);
};

#endif
