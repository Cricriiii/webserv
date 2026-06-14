#ifndef STRINGHANDLER_HPP
# define STRINGHANDLER_HPP

# include <string>
# include <cstdio>
# include <sstream>
# include <limits>
# include <cstdlib>

struct sh_count
{
	char	c;
	size_t	count;

	sh_count(): c(0), count(0) {}
};

class StringHandler
{
	public:
		StringHandler(std::string& target);
		StringHandler(const std::string& target);
		~StringHandler();

		size_t				length() const;
		void				clear();
		const char*			c_str() const;
		size_t				count(char c) const;
		size_t				count(char c, size_t n) const;
		struct sh_count*	count(const char set[], size_t size) const;



		StringHandler&	operator<<(const std::string& s);
		StringHandler&	operator<<(const char* s);
		StringHandler&	operator<<(char s);

		template<typename T>
		StringHandler&	operator<<(T n)
		{
			std::ostringstream	oss;
			oss << n;
			_ref_string.append(oss.str());
			return *this;
		}

		StringHandler&	operator+=(const std::string& s);
		StringHandler&	operator+=(const char* s);
		StringHandler&	operator+=(char s);

		template<typename T>
		StringHandler&	operator+=(T n)
		{
			StringHandler::operator<<(n);
			return *this;
		}

		template<typename T>
		T	to_number(size_t pos, bool check_end)
		{
			if (pos > _ref_string.size()) {
				throw std::logic_error("StringHandler: to_number: pos after string end");
			}

			const char*	strval = _ref_string.c_str() + pos;

			if (std::numeric_limits<long double>::max() < std::numeric_limits<T>::max()) {
				throw std::logic_error("StringHandler: to_number: Can't convert to smaller type without loss");
			}

			char		*end;
			long double	res = std::strtold(strval, &end);

			if (check_end && *end != '\0') {
				throw std::logic_error("StringHandler: to_number: Invalid bytes after value");
			}

			return static_cast<T>(res);
		}
		
	private:
		std::string&	_ref_string;
};

#endif
