#ifndef OS_HPP
# define OS_HPP

# if defined(__linux__)
#  include <linux/version.h>
#  if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#   error "epoll requires Linux >= 2.6"
#  endif

#  define LINUX	__linux__

# elif defined(__APPLE__) && defined(__MACH__)
#  include <AvailabilityMacros.h>
#  if MAC_OS_X_VERSION_MIN_REQUIRED < 1030
#   error "kqueue requires macOS >= 10.3"
#  endif

#  define APPLE	__APPLE__

# else
#  error "Unsupported OS. Requires Linux > v2.6 or Mac-OS > v10.3."

# endif
#endif
