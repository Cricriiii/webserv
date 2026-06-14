 #include "http/HTTP_Request.hpp"
 #include "tools/KeyVal_Parser.hpp"


/* ==================[constructors]========================================= */

HTTP_Request::HTTP_Request()
	:	start_line(),
		headers(),
		entity_body(),
		cookies(),
		status(0),
		socketd(0),
		body_transfer_typ(EMPTY_TRANSF),
		session(NULL)
{

}

HTTP_Request::HTTP_Request(int socketd)
	:	start_line(),
		headers(),
		entity_body(),
		cookies(),
		status(0),
		socketd(socketd),
		body_transfer_typ(EMPTY_TRANSF),
		session(NULL)

{

}


HTTP_Request::HTTP_Request(const HTTP_Request& other)
	:	start_line(other.start_line),
		headers(other.headers),
		entity_body(other.entity_body),
		cookies(other.cookies),
		status(other.status),
		socketd(other.socketd),
		body_transfer_typ(other.body_transfer_typ),
		session(other.session)
{

}

HTTP_Request::~HTTP_Request()
{

}


HTTP_Request&	HTTP_Request::operator=(const HTTP_Request& other)
{
	if (this != &other) {
		start_line = other.start_line;
		headers = other.headers;
		entity_body = other.entity_body;
		cookies = other.cookies;
		status = other.status;
		socketd = other.socketd;
		body_transfer_typ = other.body_transfer_typ;
		session = other.session;
	}
	return *this;
}

/* ==================[clear]================================================ */

void	HTTP_Request::extract_cookies()
{
	std::string		cookie_str;

	{
		cookie_cit_t	cookies = headers.find("cookie");
		if (cookies == headers.end()) { return; }

		cookie_str.append(cookies->second);
	}

	cookies = KeyVal_Parser::build_dictionary(cookie_str, "=", ";");
}





/* ==================[clear]================================================ */

void	HTTP_Request::clear()
{
	int	tmp_socket = socketd;
	reset();
	socketd = tmp_socket;
}

void	HTTP_Request::reset()
{
	*this = HTTP_Request ();
}


/* ==================[overloads]============================================ */

std::ostream&	operator<<(std::ostream& os, const HTTP_Request& request)
{
	os	<< request.start_line.method <<	' '
		<< request.start_line.URL <<	' '
		<< request.start_line.version.name << '/'
		<< request.start_line.version.major << '.'
		<< request.start_line.version.minor << "\r\n";

	for (HTTP_Request::header_cit_t	cit = request.headers.begin();
			cit != request.headers.end();
			++cit)
	{
		os	<< cit->first << ':' << cit->second << "\r\n";
	}

	os	<< "\r\n" << request.entity_body.data << "\r\n";

	return os;
}
