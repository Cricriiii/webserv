#include "HTTP_Compliance.hpp"

struct compliance_info	HTTP_Compliance::_domain_name_compliance(const std::string& hostname)
{
	size_t					pos;
	StringHandler			sh(hostname);
	struct compliance_info	compliance;

	if ((pos = hostname.find_last_of(':')) != std::string::npos) {
		if (pos == hostname.size() - 1) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}

		try {
			int port = sh.to_number<int>(pos + 1, true);
			if (port < PORT_MIN_VALUE || port > PORT_MAX_VALUE) {
				return compliance.status = E_HTTP_BAD_REQUEST, compliance;
			}
			else {
				compliance.port = port;
			}
		}
		catch (...) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
	}
	else {
		pos = hostname.size();
	}

	if (pos == 0) {
		return compliance.status = E_HTTP_BAD_REQUEST, compliance;
	}
	if (pos > 253) {
		return compliance.status = E_HTTP_BAD_REQUEST, compliance;
	}

	bool last_was_dot = true;
	for (size_t i = 0; i < pos; ++i) {
		char c = hostname[i];

		if (c == '.') {
			if (last_was_dot) {
				return compliance.status = E_HTTP_BAD_REQUEST, compliance;
			}
			last_was_dot = true;
		}
		else if (IS_DIGIT(c) || IS_ALPHA(c) || c == '-') {
			if (c == '-') {
				if (last_was_dot) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}

				if (i + 1 == pos || hostname[i + 1] == '.') {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
			}
			last_was_dot = false;
		}
		else {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
	}

	if (last_was_dot) {
		return compliance.status = E_HTTP_BAD_REQUEST, compliance;
	}

	compliance.name_type = DNS_HOSTNAME;
	compliance.domain = hostname.substr(0, pos);
	compliance.status = E_HTTP_OK;
	return compliance;
}


struct compliance_info	HTTP_Compliance::_ipv4_host_compliance(const std::string& hostname)
{
	size_t			pos;
	StringHandler	sh(hostname);
	struct compliance_info	compliance;


	if ((pos = hostname.find(':')) != std::string::npos) {

		if (hostname.find(':', pos + 1) != std::string::npos) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}

		if (pos == hostname.size() - 1) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}

		try {
			int port = sh.to_number<int>(pos + 1, true);
			if (port < PORT_MIN_VALUE || port > PORT_MAX_VALUE) {
				return compliance.status = E_HTTP_BAD_REQUEST, compliance;
			}
			else {
				compliance.port = port;
			}
		}
		catch (...) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
	}
	else {
		pos = hostname.size();
	}

	size_t total_dots = sh.count('.', pos);
	if (total_dots != 3) {
		return compliance.status = E_HTTP_BAD_REQUEST, compliance;
	}

	{
		int		current_segment_val = 0;
		int		segment_len = 0;
		int		segment_count = 0;
		bool	last_was_dot = true;

		for (size_t i = 0; i < pos; ++i) {
			char c = hostname[i];

			if (c == '.') {
				if (last_was_dot) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
				if (current_segment_val > 255) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
				current_segment_val = 0;
				segment_len = 0;
				last_was_dot = true;
				segment_count++;
			}
			else if (IS_DIGIT(c)) {
				current_segment_val = current_segment_val * 10 + (c - '0');
				segment_len++;
				last_was_dot = false;

				if (segment_len > 3) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
			}
			else {
				return compliance.status = E_HTTP_BAD_REQUEST, compliance;
			}
		}

		if (last_was_dot || current_segment_val > 255) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
		segment_count++;

		if (segment_count != 4) {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
	}

	compliance.name_type = IPV4_HOSTNAME;
	compliance.domain = hostname.substr(0, pos);
	compliance.status = E_HTTP_OK;
	return compliance;
}


struct compliance_info	HTTP_Compliance::_ipv6_host_compliance(const std::string& hostname)
{
	StringHandler			sh(hostname);
	size_t					pos;
	struct compliance_info	compliance;
	

	if ( (pos = hostname.find(']')) != std::string::npos) {
		if (hostname.at(0) == '[') {

			if ( (pos = hostname.find(']')) == std::string::npos) {
				return compliance.status = E_HTTP_BAD_REQUEST, compliance;
			}

			if (pos != hostname.size() - 1) {					
				if (hostname[pos + 1] != ':') {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}

				try {
					int port = sh.to_number<int>(pos + 2, true);
					if (port < PORT_MIN_VALUE || port > PORT_MAX_VALUE) {
						return compliance.status = E_HTTP_BAD_REQUEST, compliance;
					}
					else {
						compliance.port = port;
					}
				}
				catch (...) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
			}
			
			size_t	total_semicol = sh.count(':', pos);
			if (total_semicol < 2 || total_semicol > 8) {
				return compliance.status = E_HTTP_BAD_REQUEST, compliance;
			}

			{
				int			consecutive_semicol = 0;
				bool		double_semicol_found = false;
				int			consecutive_hex = 0;
				int			non_empty_fields = 0;
	
				for (size_t i = 1; i < pos; ++i) {
	
					char	c = hostname[i];
					bool	is_hexdig = IS_HEXDIG(c);
					bool	is_semicol = (c == ':');
	
					if (!is_hexdig && !is_semicol) {
						return compliance.status = E_HTTP_BAD_REQUEST, compliance;
					}
					else if (is_semicol) {
	
						consecutive_hex = 0;
						++consecutive_semicol;
	
						if (consecutive_semicol == 2) {
							if (double_semicol_found) {
								return compliance.status = E_HTTP_BAD_REQUEST, compliance;
							}
							else {
								double_semicol_found = true;
							}
						}
						else if (consecutive_semicol > 2) {
							return compliance.status = E_HTTP_BAD_REQUEST, compliance;
						}
					}
					else {
						consecutive_semicol = 0;
						++non_empty_fields;
	
						if (++consecutive_hex > 4) {
							return compliance.status = E_HTTP_BAD_REQUEST, compliance;
						}
					}
				}
	
				if (total_semicol != 7 && double_semicol_found == false) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
	
				if (non_empty_fields == 8 && double_semicol_found) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
			}

			for (size_t i = pos + 2; i < hostname.length(); ++i) {
				if (!IS_DIGIT(hostname[i])) {
					return compliance.status = E_HTTP_BAD_REQUEST, compliance;
				}
			}

		}
		else {
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
	}
	else {
		sh_count*	count;

		try {
			count = sh.count((char []){':', '['}, 2);
		}
		catch (std::bad_alloc &e) {
			return compliance.status = E_HTTP_INTERNAL_SERVER_ERROR, compliance;
		}

		if (count[0].count > 1 || count[1].count > 0) {
			delete count;
			return compliance.status = E_HTTP_BAD_REQUEST, compliance;
		}
	}

	compliance.name_type = IPV6_HOSTNAME;
	compliance.domain = hostname.substr(0, pos + 1);
	compliance.status = E_HTTP_OK;
	return compliance;
}

