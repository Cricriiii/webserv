#ifndef HTTP_SERIALIZER_HPP
#define HTTP_SERIALIZER_HPP

/* [PREPROCESSOR */

#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <string>

#include "conf.hpp"
#include "handlers/HandlerTypes.hpp"
#include "http/HTTP_Response.hpp"
#include "http/HTTP_Variables.hpp"
#include "tools/StringHandler.hpp"

/* [FSM */

typedef enum serializer_status {
    SET_CUR_FILE,
    BUILD_START_LINE,
    SEND_START_LINE,
    BUILD_HEADERS,
    SEND_HEADERS,
    PRE_BUILD_BODY,
    BUILD_REG_BODY,
    BUILD_CGI_BODY,
    SEND_REG_BODY,
    SEND_CGI_BODY,
    SEND_LAST_CR,
    SEND_LAST_LF,
    SERIAL_DONE
} s_status;

/* [IMPLEMENTATION] */

class HTTP_Serializer {
public:
    HTTP_Serializer(size_t buf_siz);
    HTTP_Serializer(const HTTP_Serializer& other);
    ~HTTP_Serializer();

    HTTP_Serializer& operator=(const HTTP_Serializer& other);

    void set_send_segment_siz(size_t size);

    void clear();
    void reset();

    response_status_t serialize_head(const HTTP_Response& response,
                                     resource_context_t& resource, char* target,
                                     size_t target_size);
    response_status_t serialize_body(const HTTP_Response& response,
                                     resource_context_t& resource, char* target,
                                     size_t target_size);
    void sync_cursor(ssize_t bytes_sent, size_t& cur_status);

private:
    size_t _pos;
    size_t _hd_idx;
    s_status _state;
    file_context _cur_file;
    std::string _buffer;
};

#endif
