#ifndef AUTOINDEXHANDLER_HPP
#define AUTOINDEXHANDLER_HPP

#include "handlers/IHandler.hpp"

class AutoIndexHandler : public IHandler {
public:
    AutoIndexHandler();
    ~AutoIndexHandler();

    resource_context_t execute(request_context_t& context);
};

#endif
