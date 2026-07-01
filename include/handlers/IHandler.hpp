#ifndef IHANDLER_HPP
#define IHANDLER_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "GarbageCollector.hpp"
#include "handlers/HandlerTypes.hpp"
#include "handlers/mime_types.hpp"
#include "http/HTTP_Errors.hpp"
#include "http/HTTP_Response.hpp"
#include "vserver/vserver.hpp"

class IHandler {
public:
    virtual ~IHandler() {};

    virtual resource_context_t execute(request_context_t& context) = 0;
};

#endif
