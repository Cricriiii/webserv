#include "tools/StringHandler.hpp"


/* ==================[construct, destruct]============================ */

StringHandler::StringHandler(std::string& target)
	:	_ref_string(target)
{

}

StringHandler::StringHandler(const std::string& target)
	:	_ref_string(const_cast<std::string&>(target))
{

}

StringHandler::~StringHandler()
{

}


/* ==================[member functions]=============================== */

size_t	StringHandler::length() const
{
	return _ref_string.length();
}

void	StringHandler::clear()
{
	_ref_string.clear();
}


const char*	StringHandler::c_str() const
{
	return _ref_string.c_str();
}


size_t	StringHandler::count(char c) const
{
	size_t	count = 0;

	for (std::string::const_iterator cit = _ref_string.begin(); cit != _ref_string.end(); ++cit) {
		if (*cit == c) {
			++count;
		}
	}
	return count;
}

size_t	StringHandler::count(char c, size_t n) const
{
	size_t	count = 0;

	for (std::string::const_iterator cit = _ref_string.begin(); cit != _ref_string.begin() + n; ++cit) {
		if (*cit == c) {
			++count;
		}
	}
	return count;
}

struct sh_count*		StringHandler::count(const char set[], size_t size) const
{
	struct sh_count*	count = new struct sh_count[size];
	for (size_t set_idx = 0; set_idx < size; ++set_idx) {
		count[set_idx].c = set[set_idx];
	}

	for (std::string::const_iterator cit = _ref_string.begin(); cit != _ref_string.end(); ++cit) {
		for (size_t set_idx = 0; set_idx < size; ++set_idx) {
			if (*cit == count[set_idx].c) {
				++count[set_idx].count;
			}
		}
	}
	return count;
}

/* ==================[operator<<]===================================== */

StringHandler&	StringHandler::operator<<(const std::string& s)
{
	_ref_string.append(s);

	return *this;
}

StringHandler&	StringHandler::operator<<(const char* s)
{
	if (s) {
		_ref_string.append(s);
	}
	return *this;
}

StringHandler&	StringHandler::operator<<(char s)
{
	_ref_string.push_back(s);

	return *this;
}


/* ==================[operator+=]===================================== */


StringHandler&	StringHandler::operator+=(const std::string& s)
{
	StringHandler::operator<<(s);
	return *this;
}

StringHandler&	StringHandler::operator+=(const char* s)
{
	StringHandler::operator<<(s);

	return *this;
}

StringHandler&	StringHandler::operator+=(char s)
{
	StringHandler::operator<<(s);

	return *this;
}
