#include "network/SessionManager.hpp"

SessionManager::SessionManager() {
}

SessionManager::~SessionManager() {
    for (session_map_t::iterator it = _sessions.begin(); it != _sessions.end();
         ++it) {
        delete it->second;
    }
}

SessionManager& SessionManager::get_instance() {
    static SessionManager instance;
    return instance;
}

void SessionManager::add_session(session_id_t id) {
    if (_sessions.find(id) != _sessions.end()) {
        return;
    }

    Session* new_session = new Session;
    GarbageCollector::get_instance().add_ptr(new_session, default_delete);
    new_session->id = id;
    new_session->last_activity = time(NULL);

    _sessions.insert(std::make_pair(id, new_session));
}

SessionManager::Session* SessionManager::get_session(session_id_t id) {
    session_map_t::iterator lookfor = _sessions.find(id);

    if (lookfor != _sessions.end()) {
        lookfor->second->last_activity = time(NULL);
        return lookfor->second;
    } else {
        return NULL;
    }
}

SessionManager::Session* SessionManager::retrieve_login(session_login_t login) {
    session_map_t::iterator it = _sessions.begin();

    for (; it != _sessions.end(); ++it) {
        session_data_t::iterator data = it->second->values.begin();

        for (; data != it->second->values.end(); ++data) {
            if (it->first == login) {
                return it->second;
            }
        }
    }
    return NULL;
}

void SessionManager::clean_expired(time_t timeout_seconds) {
    time_t now = time(NULL);

    std::vector<session_id_t> to_delete;

    session_map_t::iterator it = _sessions.begin();

    for (; it != _sessions.end(); ++it) {
        if (now - it->second->last_activity > timeout_seconds) {
            to_delete.push_back(it->first);
        }
    }

    for (std::vector<session_id_t>::iterator it = to_delete.begin();
         it != to_delete.end(); ++it) {
        delete _sessions.find(*it)->second;
        _sessions.erase(*it);
    }
}

SessionManager::session_id_t SessionManager::generate_session_id() {
    std::string new_id;

    while (true) {
        new_id.clear();
        for (int i = 0; i < SESSION_ID_SIZ; ++i) {
            new_id +=
                session_id_charset[rand() % (sizeof(session_id_charset) - 1)];
        }
        if (_sessions.find(new_id) == _sessions.end())
            break;
    }

    return new_id;
}
