#ifndef SESSIONMANAGER_HPP
# define SESSIONMANAGER_HPP

# include <string>
# include <map>
# include <vector>
# include <ctime>
# include <cstdlib>

# include "GarbageCollector.hpp"

# include "config_constants.hpp"

class SessionManager
{
public:
	typedef std::string							session_id_t;
	typedef std::string							session_login_t;
	typedef std::map<std::string, std::string>	session_data_t;
	
	struct Session {
		session_id_t	id;
		session_data_t	values;
		time_t			last_activity;

		Session(): id(), values(), last_activity(0) {}
	};

	static SessionManager&	get_instance();

	void			add_session(session_id_t id);
	Session*		get_session(session_id_t id);
	Session*		retrieve_login(session_login_t login);
	void			clean_expired(time_t timeout_seconds);
	session_id_t	generate_session_id();

private:
	typedef std::map<session_id_t, Session*>    session_map_t;

	session_map_t	_sessions;

	SessionManager();
	~SessionManager();
	SessionManager(const SessionManager&);
	SessionManager& operator=(const SessionManager&);
};

#endif
