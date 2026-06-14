#ifndef HTTP_Request_HPP
# define HTTP_Request_HPP


/* ==================[PREPROCESSOR]========================================= */

# include <iostream>
# include <string>
# include <map>

# include <network/SessionManager.hpp>





/* ==================[ENUMS]=============================================== */

typedef enum data_transfer_type {
	EMPTY_TRANSF	= 0,
	CHUNKED_TRANSF	= (1 << 0),
	LENGTH_TRANSF	= (1 << 1)
} data_transfer_type;





/* ==================[IMPLEMENTATION]====================================== */

class	HTTP_Request
{
	public:
	
	/* --------------[typedefs]----------------------------------- */
	
		typedef std::map<std::string, std::string>	header_t;
		typedef header_t::iterator					header_it_t;
		typedef header_t::const_iterator			header_cit_t;
		
		typedef header_t							cookie_t;
		typedef header_it_t							cookie_it_t;
		typedef header_cit_t						cookie_cit_t;

	/* --------------[constructors]------------------------------- */

		HTTP_Request();
		HTTP_Request(int socketd);
		HTTP_Request(const HTTP_Request& other);
		~HTTP_Request();

		HTTP_Request&	operator=(const HTTP_Request& other);


	/* --------------[functions]---------------------------------- */
	
		void	extract_cookies();
		void	clear();
		void	reset();

		
	/* --------------[variables]---------------------------------- */

		struct http_req_start_line {
			std::string		method;
			std::string		URL;
			struct http_req_version {
				std::string	name;
				int			major;
				int			minor;
				http_req_version(): name("HTTP"), major(0), minor(0) {}
			} 				version;
			http_req_start_line(): method(), URL(), version() {}
		}									start_line;
		header_t							headers;
		struct http_req_entity_body {
			std::string						data;
			size_t							size;
			http_req_entity_body(): data(), size(0) {}
		}									entity_body;

		cookie_t							cookies;

		int 								status;
		int									socketd;

		data_transfer_type					body_transfer_typ;

		SessionManager::Session*			session;
};


/* ==================[OVERLOADS]============================================ */

std::ostream&	operator<<(std::ostream& os, const HTTP_Request& request);

#endif
