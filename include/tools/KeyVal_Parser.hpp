#ifndef KEYVALPARSER_HPP
# define KEYVALPARSER_HPP

# include <string>
# include <map>
# include <stdexcept>
# include <cstring>

enum match_mode { CHARSET, LITERAL };

namespace KeyVal_Parser {
	std::map<std::string, std::string>	build_dictionary(const std::string& ref_string,
														const char* equality,
														const char* separator,
														match_mode equ_mode = CHARSET,
														match_mode sep_mode = CHARSET);
};

#endif
