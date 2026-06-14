#ifndef NETCONNECTION_HPP
# define NETCONNECTION_HPP


/* ==================[PREPROCESSOR]===================================================================================================== */

# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>

# include <string>
# include <algorithm>

# include "GarbageCollector.hpp"

# include "http/HTTP_Request.hpp"
# include "http/HTTP_Parser.hpp"
# include "http/HTTP_Response.hpp"
# include "http/HTTP_Serializer.hpp"
# include "http/HTTP_Errors.hpp"
# include "http/HTTP_Variables.hpp"

# include "handlers/HandlerTypes.hpp"
# include "handlers/HandlerValidator.hpp"

# include "network/SessionManager.hpp"






/* ==================[TYPEDEFS]========================================================================================================= */

typedef struct response	nm_response;
typedef struct conf_http	conf_http_t;





/* ==================[ENUMS]============================================================================================================ */

typedef enum polling_state
{
	S_POLL_NONE,
	S_POLL_IN,
	S_POLL_OUT,
	S_POLL_IN_OUT,
	S_POLL_ERR
} poll_state_t;



typedef enum e_conn_state
{
	S_CONN_WAIT				= (1 << 0),
	S_CONN_READ				= (1 << 1),
	S_CONN_HANDLE			= (1 << 2),
	S_CONN_WRITE_TO_CGI		= (1 << 3),
	S_CONN_CGI_WAIT			= (1 << 4),
	S_CONN_DLOAD_CGI		= (1 << 5),
	S_CONN_DLOAD_BODY		= (1 << 6),
	S_CONN_BUILD_HEAD		= (1 << 7),
	S_CONN_SEND_HEAD		= (1 << 8),
	S_CONN_SEND_BODY		= (1 << 9),
	S_CONN_CLI_CLOSE		= (1 << 10)
} conn_state_t;

inline conn_state_t	operator|(conn_state_t lhs, conn_state_t rhs) {
	return static_cast<conn_state_t>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline conn_state_t&	operator|=(conn_state_t& lhs, conn_state_t rhs) {
	return lhs = lhs | rhs;
}



typedef enum e_poll_operations
{
	SCAN_EVENT_IN		= (1 << 0),
	SCAN_EVENT_OUT		= (1 << 1),
	SCAN_EVENT_ERR		= (1 << 2)
} poll_op_t;

inline poll_op_t	operator|(poll_op_t lhs, poll_op_t rhs)
{
	return static_cast<poll_op_t>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline poll_op_t	operator|=(poll_op_t lhs, poll_op_t rhs)
{
	return lhs = lhs | rhs;
}



typedef enum e_cgi_exit_state {
	CGI_WAIT_ERR,
	CGI_EXEC_ON,
	CGI_EXIT_OK,
	CGI_EXEC_FAIL
} cgi_exit_state_t;





/* ==================[IMPLEMENTATION]=================================================================================================== */

class NetConnection
{
	
/* --------------[constructors]----------------------------------------------------------------- */

public:
	NetConnection(int socketd, const conf_http_t& http_conf, size_t obuf_siz);
	NetConnection(const NetConnection &other);
	~NetConnection();
	NetConnection&	operator=(const NetConnection& other);

private:
	NetConnection();



/* --------------[private variables]------------------------------------------------------------ */

private:
	int					_socketd;
	poll_state_t		_poll_state;
	conn_state_t		_state;
	bool				_state_change;
	HTTP_Request		_request;
	HTTP_Response		_response;
	HTTP_Parser			_parser;
	HTTP_Serializer		_serializer;
	request_context_t	_context;

	bool				_keep_alive;
	bool				_kill;

	IHandler*			_handler;
	resource_context_t	_resource;
	size_t			 	_total_payload_progress;

	bool				_in_ready_list;

	SessionManager::Session*	_session;
	bool						_is_new_session;

	long long			_cgi_start_time;

	bool				_wait_before_next_io;



/* --------------[parse request]---------------------------------------------------------------- */

public:
	size_t	read_request();
	size_t	read_request(const char* buffer, size_t size);

private:
	SessionManager::Session*	_init_session();
	bool						_retrieve_session();



/* --------------[handle request]--------------------------------------------------------------- */

public:
	void	handle_request();
	void	load_context(const request_context_t& context);
	void	feed_cgi();

private:
	void	_define_close_behavior();
	void	_execute_request();
	void	_flush_post_header_buffer();
	void	_flush_body_into_file();



/* --------------[save request body]------------------------------------------------------------ */

public:
	bool	save_body(char* buffer, ssize_t* buffer_size);

private:
	bool	_save_content_body(char* buffer, ssize_t* buffer_size);
	bool	_save_chunk_body(char* buffer, ssize_t* buffer_size);



/* --------------[build headers]---------------------------------------------------------------- */

public:
	void	build_headers();

private:
	void	_create_header(const char* key, const char* val);
	void	_append_header(const char* key, const char* val);

	void	_add_header_if_session();
	void	_add_header_if_length();



public:
	response_status_t	head_stream(char* buffer, ssize_t capacity);
	response_status_t	body_stream(char* buffer, ssize_t capacity);
	void				sync_cursor(ssize_t bytes_sent, size_t& cur_status);



/* --------------[common gateway interface]----------------------------------------------------- */

public:
	bool		read_cgi_output(char* buffer, ssize_t* buffer_read, size_t buffer_capacity);
	bool		is_cgi() const;
	int			wait_cgi();
	long long	cgi_start_time() const;
	int			kill_cgi();

	void		set_server_err(http_status_t error_status);

private:
	void		_parse_cgi_headers();
	void		_try_new_session(HTTP_Request::header_t& header_list);



/* --------------[getters, setters]------------------------------------------------------------- */

public:
	int				socketd() const;

	void			set_poll_state(poll_state_t state);
	poll_state_t	poll_state();

	void			set_state(conn_state_t state);
	conn_state_t	state();
	bool			state_change();
	void			reset_state_change();

	const HTTP_Request&		request() const;
	const HTTP_Response&	response() const;

	bool	ibuffer_isdata(void) const;

	bool	keep_alive() const;
	bool	kill() const;

	IHandler*	handler();

	bool	wait_before_next_io() const;
	void	set_wait_before_next_io(bool state);



/* --------------[config - request / response]-------------------------------------------------- */

public:
	void	set_send_segment_siz(size_t size);

	size_t	http_max_method_siz() const;
	void	set_http_max_method_siz(size_t size);

	size_t	http_max_uri_siz() const;
	void	set_http_max_uri_siz(size_t size);

	size_t	http_max_header_key_siz() const;
	void	set_http_max_header_key_siz(size_t size);

	size_t	http_max_header_val_siz() const;
	void	set_http_max_header_val_siz(size_t size);

	size_t	http_max_header_amt_siz() const;
	void	set_http_max_header_amt_siz(size_t size);

	size_t	http_max_body_siz() const;
	void	set_http_max_body_siz(size_t size);

	bool	ready() const;
	void	set_readiness(bool state);

	HTTP_Request&	request();
	HTTP_Response&	response();



/* --------------[clearing functions]----------------------------------------------------------- */

public:
	void	clear_request();
	void	clear_response();
	void	clear_parser();
	void	clear_resource();
	void	clear();
	void	reset();
};

#endif
