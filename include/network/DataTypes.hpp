#ifndef DataTypes_HPP
# define DataTypes_HPP

# include <cstddef>

struct buffer {
	size_t	capacity;
	ssize_t	size;
	char*	data;

	buffer(): capacity(0), size(0), data(NULL) {}
};

#endif
