#ifndef GETTIMENOW_HPP
# define GETTIMENOW_HPP

# include <sys/time.h>

static long long	getTimeNow()
{
	timeval tv;
	gettimeofday(&tv, 0);
	return static_cast<long long>(tv.tv_sec) * 1000000LL + tv.tv_usec;
}

#endif
