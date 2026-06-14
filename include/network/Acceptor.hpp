#ifndef ACCEPTOR_HPP
# define ACCEPTOR_HPP

/* ==================[INCLUDES]=========================================== */

# include <stdexcept>

# include <fcntl.h>
# include <unistd.h>

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

# include "GarbageCollector.hpp"


/* ==================[IMPLEMENTATION]======================================= */

typedef enum protocol_type
{
	PROTO_IPV4,
	PROTO_IPV6,
} protocol_type;


class Acceptor
{

public:
/* --------------[constructors]------------------------------- */
	Acceptor(int port, protocol_type protocol);
	~Acceptor();

/* --------------[implementation]----------------------------- */
	int		accept();
	int		socketd() const;
	void	close();


private:
	int			_socketd;
	int			_port;


/* --------------[disabled]----------------------------------- */
private:
	Acceptor();
	Acceptor(const Acceptor &other);
	Acceptor&	operator=(const Acceptor& other);

};

#endif
