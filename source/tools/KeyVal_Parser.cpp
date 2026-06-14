#include "tools/KeyVal_Parser.hpp"
#include "tools/char_tools.hpp"

static bool _in_set(char tested, const char* set)
{
	while (*set) {
		if (tested == *set++) return true;
	}
	return false;
}

static bool _common_values(const char* equality_set, const char* separator_set)
{
	while (*equality_set) {
		if (_in_set(*equality_set++, separator_set)) return true;
	}
	return false;
}

__attribute__((nonnull(2, 3)))
std::map<std::string, std::string>
KeyVal_Parser::build_dictionary(const std::string& ref_string,
								const char* equality,
								const char* separator,
								match_mode equ_mode,
								match_mode sep_mode)
{
	using namespace std;

	if ( (equ_mode == CHARSET || sep_mode == CHARSET) && _common_values(equality, separator)) {
		throw logic_error("KeyVal_Parser::build_dictionary: Common values in equality and separator");
	}

	map<string, string>	result;
	string				token;
	string				key;
	size_t				equ_len = strlen(equality);
	size_t				sep_len = strlen(separator);

	for (size_t i = 0; i < ref_string.length();) {

		bool matched_equ = (equ_mode == CHARSET)
			? _in_set(ref_string[i], equality)
			: ref_string.compare(i, equ_len, equality) == 0;

		bool matched_sep = (sep_mode == CHARSET)
			? _in_set(ref_string[i], separator)
			: ref_string.compare(i, sep_len, separator) == 0;

		if (matched_equ) {
			char_tools::trim_string(token, ' ');
			result[token] = "";
			key.swap(token);
			token.clear();
			i += (equ_mode == CHARSET) ? 1 : equ_len;
		}
		else if (matched_sep) {
			char_tools::trim_string(token, ' ');
			result[key] = token;
			token.clear();
			i += (sep_mode == CHARSET) ? 1 : sep_len;
		}
		else {
			token += ref_string[i];
			++i;
		}
	}

	char_tools::trim_string(token, ' ');
	result[key] = token;

	return result;
}
