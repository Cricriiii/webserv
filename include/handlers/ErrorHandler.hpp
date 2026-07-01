#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "handlers/IHandler.hpp"
#include "vserver/vserver.hpp"

class ErrorHandler : public IHandler {
private:
    int _code;

public:
    ErrorHandler(int code);
    ~ErrorHandler();

    resource_context_t execute(request_context_t& context);

    /* --------------[static functions]---------------------------------- */

    static inline ErrorHandler* ErrorHandler_ptr(IHandler* handler) {
        return dynamic_cast<ErrorHandler*>(handler);
    }

    static inline bool isErrorHandler(IHandler* handler) {
        return ErrorHandler_ptr(handler) != NULL;
    }
};

#endif
