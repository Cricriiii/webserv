# include "conf.hpp"
# include <cstddef>
# include <string>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdexcept>
# include <cerrno>
# include <cstdlib>





static bool verif_path_root(const std::string& path)
{
	if (path.size() < 2 || path[0] != '.' || path[1] != '/')
		return false;

	for (size_t i = 2; i < path.size(); ++i)
	{
		if (path[i] == '/' && path[i - 1] == '/')
			return false;
	}

	return true;
}

static bool verif_path(const std::string& path)
{
	if (path.empty() || path[0] != '/')
		return false;

	for (size_t i = 1; i < path.size(); ++i)
	{
		if (path[i] == '/' && path[i - 1] == '/')
			return false;
	}

	return true;
}





void	Server_configuration::parsing()
{
	if (_tokens.empty())
		throw std::invalid_argument(".conf: empty file");

	_i_token = 0;
	if (!is_token(_i_token, "http"))
		throw std::invalid_argument(msg_error("expected 'http'", _i_token));

	++_i_token;
	if (is_end_token(_i_token) || !has_status(_i_token, OPEN_BLOCK))
		throw std::invalid_argument(msg_error("expected '{' after 'http'", _i_token - 1));

	init_conf_http();

	++_i_token;
	while (!is_end_token(_i_token) && !has_status(_i_token, CLOSE_BLOCK))
	{
		if (is_token(_i_token, "client_http_max_body_siz") || is_token(_i_token, "recv_segment_siz") || is_token(_i_token, "http_max_header_amt_siz")
			|| is_token(_i_token, "send_segment_siz") || is_token(_i_token, "http_max_method_siz") || is_token(_i_token, "http_max_uri_siz")
			|| is_token(_i_token, "http_max_header_key_siz") || is_token(_i_token, "http_max_header_val_siz") || is_token(_i_token, "http_max_body_siz"))
		{
			parse_value_http();
		}
		
		else if (is_token(_i_token, "server"))
		{
			if (is_end_token(_i_token + 1) || !has_status(_i_token + 1, OPEN_BLOCK))
				throw std::invalid_argument(msg_error("expected '{' after 'server'", _i_token));

			parsing_server();
		}

		else
			throw std::invalid_argument(msg_error("invalid token", _i_token));
	}
	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("expected token '}' after end of http block", _i_token - 1));

	++_i_token;
	if (_i_token != _tokens.size())
		throw std::invalid_argument(msg_error("unexpected token after end of http block", _i_token));
}





void	Server_configuration::parsing_server()
{
	vserver_t	server;

	init_vserver(server);
	_i_token += 2;

	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("invalid structure for server block", _i_token - 1));

	while (!is_end_token(_i_token) && !has_status(_i_token, CLOSE_BLOCK))
	{
		if (is_token(_i_token, "listen") || is_token(_i_token, "root") || is_token(_i_token, "max_body_size"))
			parse_single_value_directive(server);

		else if (is_token(_i_token, "index") || is_token(_i_token, "server_name"))
			parse_multi_value_directive(server);

		else if (is_token(_i_token, "error_page"))
			parse_error_page_directive(server);

		else if (is_token(_i_token, "location"))
			parse_location_block(server);

		else
			throw std::invalid_argument(msg_error("invalid token", _i_token));
	}
	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("expected '}' after server block", _i_token - 1));

	if (verif_server_conflict(server) == true)
		throw std::invalid_argument(msg_error("conflict server_name and port", _i_token));

	_vservers.push_back(server);
	++_i_token;
}





void	Server_configuration::parse_value_http()
{
	size_t	start = _i_token;

	if (is_end_token(start))
		throw std::invalid_argument(msg_error("invalid structure", start));

	++_i_token;
	if (is_end_token(_i_token) || !has_status(_i_token, WORD))
		throw std::invalid_argument(msg_error("invalid structure", start));

	++_i_token;
	if (is_end_token(_i_token) || !has_status(_i_token, SEMICOLON))
		throw std::invalid_argument(msg_error("expected ';' after simple guidelines", start));

	else if (is_token(start, "recv_segment_siz"))
	{
		if (_conf_http.recv_segment_siz != -1)
			throw std::invalid_argument(msg_error("recv_segment_siz defined a second time", start));
		_conf_http.recv_segment_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "send_segment_siz"))
	{
		if (_conf_http.send_segment_siz != -1)
			throw std::invalid_argument(msg_error("send_segment_siz defined a second time", start));
		_conf_http.send_segment_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "http_max_method_siz"))
	{
		if (_conf_http.http_max_method_siz != -1)
			throw std::invalid_argument(msg_error("http_max_method_siz defined a second time", start));
		_conf_http.http_max_method_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "http_max_uri_siz"))
	{
		if (_conf_http.http_max_uri_siz != -1)
			throw std::invalid_argument(msg_error("http_max_uri_siz defined a second time", start));
		_conf_http.http_max_uri_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "http_max_header_key_siz"))
	{
		if (_conf_http.http_max_header_key_siz != -1)
			throw std::invalid_argument(msg_error("http_max_header_key_siz defined a second time", start));
		_conf_http.http_max_header_key_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "http_max_header_val_siz"))
	{
		if (_conf_http.http_max_header_val_siz != -1)
			throw std::invalid_argument(msg_error("http_max_header_val_siz defined a second time", start));
		_conf_http.http_max_header_val_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "http_max_header_amt_siz"))
	{
		if (_conf_http.http_max_header_amt_siz != -1)
			throw std::invalid_argument(msg_error("http_max_header_amt_siz defined a second time", start));
		_conf_http.http_max_header_amt_siz = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "http_max_body_siz"))
	{
		if (_conf_http.http_max_body_siz != -1)
			throw std::invalid_argument(msg_error("http_max_body_siz defined a second time", start));
		_conf_http.http_max_body_siz = strtoll(get_token(start + 1), start + 1);
	}
	++_i_token;
}





void	Server_configuration::parse_single_value_directive(vserver_t& server)
{
	size_t	start = _i_token;

	if (is_end_token(start))
		throw std::invalid_argument(msg_error("invalid structure", start));

	++_i_token;
	if (is_end_token(_i_token) || !has_status(_i_token, WORD))
		throw std::invalid_argument(msg_error("invalid structure", start));

	++_i_token;
	if (is_end_token(_i_token) || !has_status(_i_token, SEMICOLON))
		throw std::invalid_argument(msg_error("expected ';' after simple guidelines", start));

	else if (is_token(start, "listen"))
	{
		if (server.port != -1)
			throw std::invalid_argument(msg_error("listen defined a second time", start));
		server.port = strtoi(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "max_body_size"))
	{
		if (server.max_body_size != -1)
			throw std::invalid_argument(msg_error("max_body_size defined a second time", start));
		server.max_body_size = strtoll(get_token(start + 1), start + 1);
	}

	else if (is_token(start, "root"))
	{
		if (server.root.empty() == false)
			throw std::invalid_argument(msg_error("root defined a second time", start));


		if (verif_path_root(get_token(start + 1)) == false)
			throw std::invalid_argument(msg_error("invalid root path", start + 1));
		server.root = get_token(start + 1);
	}

	++_i_token;
}



void	Server_configuration::parse_multi_value_directive(vserver_t& server)
{
	size_t	start = _i_token;

	if (is_end_token(start + 1) || !has_status(start + 1, WORD))
		throw std::invalid_argument(msg_error("invalid structure", start));

	if (is_token(start, "index") && !server.index.empty())
		throw std::invalid_argument(msg_error("index defined a second time", start));

	if (is_token(start, "server_name") && !server.name.empty())
		throw std::invalid_argument(msg_error("server_name defined a second time", start));

	_i_token = start + 1;
	while (!is_end_token(_i_token))
	{
		if (!has_status(_i_token, WORD) && !has_status(_i_token, SEMICOLON))
			throw std::invalid_argument(msg_error("expected ';' after simple guidelines", _i_token));

		if (has_status(_i_token, SEMICOLON))
			break;

		if (is_token(start, "index"))
			server.index.push_back(get_token(_i_token));

		else if (is_token(start, "server_name"))
			if (server.name.insert(get_token(_i_token)).second == false)
				throw std::invalid_argument(msg_error("server_name defined a second time in the same server", _i_token));

		++_i_token;
	}

	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("unexpected end of file", start));
	++_i_token;
}



void	Server_configuration::parse_error_page_directive(vserver_t& server)
{
	size_t			start = _i_token;
	std::set<int>	error_codes;

	if (is_end_token(start + 1) || !has_status(start + 1, WORD) || !has_status(start + 2, WORD))
		throw std::invalid_argument(msg_error("invalid structure", start));

	++_i_token;

	while (!is_end_token(_i_token))
	{
		if (!has_status(_i_token, WORD) && !has_status(_i_token, SEMICOLON))
			throw std::invalid_argument(msg_error("expected ';' after simple guidelines", _i_token));

		if (has_status(_i_token, WORD) && has_status(_i_token + 1, SEMICOLON))
			break;

		if (error_codes.insert(strtoi(get_token(_i_token), _i_token)).second == false)
			throw std::invalid_argument(msg_error("error_page defined a second time for the same error code", _i_token));

		++_i_token;
	}

	for (std::set<int>::const_iterator it = error_codes.begin(); it != error_codes.end(); ++it)
	{
		if (server.errors.find(*it) != server.errors.end())
			throw std::invalid_argument(msg_error("error_page defined a second time for the same error code", start));

		server.errors[*it] = get_token(_i_token);
	}
	_i_token += 2;

	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("unexpected end of file", start));
}





void	Server_configuration::parse_location_block(vserver_t& server)
{
	size_t		start = _i_token;
	location_t	location;
	location.autoindex = false;
	location.cgi = false;

	if (is_end_token(start + 2))
		throw std::invalid_argument(msg_error("invalid structure for location", start));

	if (!has_status(start + 1, WORD))
		throw std::invalid_argument(msg_error("invalid structure for location", start));

	if (verif_location_path(server, get_token(start + 1)) == true)
		throw std::invalid_argument(msg_error("location defined a second time for the same path", start + 1));

	if (verif_path(get_token(start + 1)) == false)
		throw std::invalid_argument(msg_error("invalid location path", start + 1));

	location.path = get_token(start + 1);

	if (!has_status(start + 2, OPEN_BLOCK))
		throw std::invalid_argument(msg_error("expected '{' after location path", start + 1));

	_i_token = start + 3;
	while (!is_end_token(_i_token) && !has_status(_i_token, CLOSE_BLOCK))
	{
		if (is_token(_i_token, "root") || is_token(_i_token, "autoindex") || is_token(_i_token, "cgi_extension")
		|| is_token(_i_token, "cgi_path") || is_token(_i_token, "cgi"))
			parse_single_value_directive_location(location);

		else if (is_token(_i_token, "limit_except") || is_token(_i_token, "index"))
			parse_multi_value_directive_location(location);

		else
			throw std::invalid_argument(msg_error("invalid token in location block", _i_token));
	}

	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("unexpected end of file in location block", start));

	server.location.push_back(location);
	++_i_token;
}



void	Server_configuration::parse_single_value_directive_location(location_t& location)
{
	size_t	start = _i_token;

	if (is_end_token(start + 2))
		throw std::invalid_argument(msg_error("invalid structure", start));

	if (!has_status(start + 1, WORD))
		throw std::invalid_argument(msg_error("invalid structure", start));

	if (!has_status(start + 2, SEMICOLON))
		throw std::invalid_argument(msg_error("expected ';' after simple guidelines", start));

	if (is_token(start, "root"))
	{
		if (location.root.empty() == false)
			throw std::invalid_argument(msg_error("root defined a second time", start));

		if (verif_path_root(get_token(start + 1)) == false)
			throw std::invalid_argument(msg_error("invalid root path", start + 1));
		location.root = get_token(start + 1);
	}

	else if (is_token(start, "autoindex"))
	{
		if (location.autoindex == true)
			throw std::invalid_argument(msg_error("autoindex defined a second time", start));
		if (is_token(start + 1, "on"))
			location.autoindex = true;
	}

	else if (is_token(start, "cgi"))
	{
		if (location.cgi == true)
			throw std::invalid_argument(msg_error("cgi defined a second time", start));
		if (is_token(start + 1, "on"))
			location.cgi = true;
	}

	else if (is_token(start, "cgi_extension"))
	{
		if (location.cgi_extension.empty() == false)
			throw std::invalid_argument(msg_error("cgi_extension defined a second time", start));

		if (get_token(start + 1).empty() || get_token(start + 1)[0] != '.')
			throw std::invalid_argument(msg_error("invalid cgi_extension", start + 1));
		location.cgi_extension = get_token(start + 1);
	}
	
	else if (is_token(start, "cgi_path"))
	{
		if (location.cgi_path.empty() == false)
			throw std::invalid_argument(msg_error("cgi_path defined a second time", start));

		if (verif_path(get_token(start + 1)) == false)
			throw std::invalid_argument(msg_error("invalid cgi_path", start + 1));
		location.cgi_path = get_token(start + 1);
	}

	_i_token = start + 3;
}



void	Server_configuration::parse_multi_value_directive_location(location_t& location)
{
	size_t	start = _i_token;

	if (is_end_token(start + 1) || !has_status(start + 1, WORD))
		throw std::invalid_argument(msg_error("invalid structure", start));

	if (is_token(start, "index") && !location.index.empty())
		throw std::invalid_argument(msg_error("index defined a second time", start));

	if (is_token(start, "limit_except") && !location.limit_except.empty())
		throw std::invalid_argument(msg_error("limit_except defined a second time", start));

	_i_token = start + 1;
	while (!is_end_token(_i_token))
	{
		if (!has_status(_i_token, WORD) && !has_status(_i_token, SEMICOLON))
			throw std::invalid_argument(msg_error("expected ';' after simple guidelines", _i_token));

		if (has_status(_i_token, SEMICOLON))
			break;

		if (is_token(start, "index"))
			location.index.push_back(get_token(_i_token));

		if (is_token(start, "limit_except"))
		{
			if (location.limit_except.insert(get_token(_i_token)).second == false)
				throw std::invalid_argument(msg_error("method defined a second time in the same location", _i_token));
		}

		++_i_token;
	}

	if (is_end_token(_i_token))
		throw std::invalid_argument(msg_error("unexpected end of file", start));
	++_i_token;
}
