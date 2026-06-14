#ifndef HTTP_Response_HPP
# define HTTP_Response_HPP


/* ==================[PREPROCESSOR]========================================= */

# include <iostream>
# include <cstddef>
# include <string>
# include <map>

# include "http/HTTP_ASCII.hpp"

# include "tools/StringHandler.hpp"




/* ==================[IMPLEMENTATION]======================================= */

class	HTTP_Response
{
public:

/* --------------[typedefs]----------------------------------- */

typedef std::map<std::string, std::string>	header_t;
typedef header_t::iterator					header_it_t;
typedef header_t::const_iterator			header_cit_t;


/* --------------[constructors]------------------------------- */
	HTTP_Response();
	HTTP_Response(const HTTP_Response& other);
	~HTTP_Response();
	
	HTTP_Response&	operator=(const HTTP_Response& other);


/* --------------[functions]---------------------------------- */

	void	clear();


/* --------------[response variables]------------------------- */

	struct http_resp_start_line {
		struct http_resp_version {
			std::string	name;
			int			major;
			int			minor;
			http_resp_version(): name("HTTP"), major(0), minor(0) {}
		} 				version;
		struct http_resp_status {
			int			code;
			std::string	reason;
			http_resp_status(): code(0), reason() {}
		}				status;
		http_resp_start_line(): version(), status() {}
	}									start_line;
	std::map<std::string, std::string>	headers;
	struct http_resp_entity_body {
		std::string						data;
		size_t							size;
		http_resp_entity_body(): data(), size(0) {}
	}									entity_body;
	size_t								raw_bytes_sent;
	size_t								size;


private:

};


/* ==================[OVERLOADS]============================================ */

std::ostream&	operator<<(std::ostream& os, const HTTP_Response& response);
StringHandler&	operator<<(StringHandler& sb, const HTTP_Response& response);

#endif
