#ifndef HEXDUMP_HPP
# define HEXDUMP_HPP

# include <iostream>
# include <sstream>
# include <iomanip>

# define HEXDUMP_WIDTH	16
# define LINE_NUM_WIDTH	8
# define CHAR_WIDTH		2
# define SP				' '

void	hexdump(const char* c_string);
void	hexdump(const char* c_string, size_t size);

#endif
