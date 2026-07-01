#include "network/NetworkManager.hpp"

#include "tools/define_size.hpp"
#include "tools/getTimeNow.hpp"
#include "tools/hexdump.hpp"

/* [CONSTRUCTORS] */

static void _allocate_buffer(buffer& buffer, size_t siz) {
    buffer.data = new char[siz];
    buffer.capacity = siz;

    GarbageCollector::get_instance().add_ptr(buffer.data, array_delete);
}

void NetworkManager::_new_acceptor(int port, protocol_type proto) {
    Acceptor* a = new Acceptor(port, proto);
    _acceptors[a->socketd()] = a;

    GarbageCollector::get_instance().add_ptr(a, default_delete);
}

NetworkManager::NetworkManager(vserver_list& vserv,
                               const http_lport_list& lports,
                               const conf_http_t& http_conf)
    : _http_conf(http_conf),
      _event_fd(-1),
      _acceptors(),
      _router(vserv),
      _connections(),
      _ready_lst(),
      _ready_del_lst(),
      _close_lst(),
      _events(),
      _ibuffer(),
      _obuffer() {
    try {
        for (http_lport_list::iterator it = lports.begin(); it != lports.end();
             ++it) {
            _new_acceptor(*it, PROTO_IPV4);
            _new_acceptor(*it, PROTO_IPV6);
        }

        _allocate_buffer(_ibuffer, define_size(_DFLT_RECV_SEGMENT_SIZ,
                                               _http_conf.recv_segment_siz));
        _allocate_buffer(_obuffer, define_size(_DFLT_SEND_SEGMENT_SIZ,
                                               _http_conf.send_segment_siz));
    } catch (std::exception& e) {
        for (acceptor_list::iterator it = _acceptors.begin();
             it != _acceptors.end(); ++it) {
            delete it->second;
        }

        delete _ibuffer.data;
        delete _obuffer.data;
        throw;
    }

    _init_event_polling();
}

NetworkManager::~NetworkManager() {
    close(_event_fd);

    delete[] _ibuffer.data;
    delete[] _obuffer.data;

    for (acceptor_list::iterator it = _acceptors.begin();
         it != _acceptors.end(); ++it) {
        delete it->second;
    }

    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        close(it->second.socketd());
    }
}

/* [REACTOR - CORE LOOP] */

static void _apply_poll_event_state(NetConnection& cur_conn) {
    switch (cur_conn.poll_state()) {
        case S_POLL_NONE:
            cur_conn.set_state(S_CONN_WAIT);
            break;

        case S_POLL_ERR:
            cur_conn.set_state(S_CONN_CLI_CLOSE);
            break;

        case S_POLL_IN:
            if (cur_conn.state() == S_CONN_WAIT) {
                cur_conn.set_state(S_CONN_READ);
            }
            break;

        case S_POLL_IN_OUT:
        case S_POLL_OUT:
            break;
    }
}

void NetworkManager::_conn_read(NetConnection& cur_conn) {
    if (cur_conn.wait_before_next_io()) {
        cur_conn.set_wait_before_next_io(false);
    } else {
        cur_conn.set_wait_before_next_io(true);

        if (cur_conn.ibuffer_isdata()) {
            cur_conn.read_request();
        } else if ((_ibuffer.size = recv(cur_conn.socketd(), _ibuffer.data,
                                         _ibuffer.capacity, 0)) > 0) {
            cur_conn.read_request(_ibuffer.data, _ibuffer.size);
        } else if (_ibuffer.size == 0) {
            cur_conn.set_state(S_CONN_CLI_CLOSE);
        } else {
            _ready_del_lst.push_back(cur_conn.socketd());
        }
    }
}

void NetworkManager::_conn_handle(NetConnection& cur_conn) {
    try {
        cur_conn.load_context(_router.route(cur_conn.request()));
        cur_conn.handle_request();
    } catch (...) {
        cur_conn.set_server_err(E_HTTP_INTERNAL_SERVER_ERROR);
        cur_conn.set_state(S_CONN_HANDLE);
    }
}

void NetworkManager::_conn_write_to_cgi(NetConnection& cur_conn) {
    cur_conn.feed_cgi();

    cur_conn.set_state(S_CONN_CGI_WAIT);
}

bool NetworkManager::_cgi_kill_if_timeout(NetConnection& cur_conn,
                                          long long timeout_time) {
    long long tn = getTimeNow();
    long long st = cur_conn.cgi_start_time();
    long long diff = tn - st;

    if (diff >= timeout_time) {
        return cur_conn.kill_cgi() == 0;
    } else {
        return false;
    }
}

void NetworkManager::_conn_cgi_wait(NetConnection& cur_conn) {
    int cgi_state = cur_conn.wait_cgi();
    http_status_t http_status = E_HTTP_OK;

redispatch:
    switch (cgi_state) {
        case CGI_EXEC_ON:
            if (!_cgi_kill_if_timeout(cur_conn, CGI_TIMEOUT)) {
                _mod_client_interest_set(cur_conn.socketd(), SCAN_EVENT_OUT);
                break;
            } else {
                cgi_state = CGI_EXEC_FAIL;
                http_status = E_HTTP_GATEWAY_TIMEOUT;
                goto redispatch;
            }

        case CGI_EXIT_OK:
            cur_conn.set_state(S_CONN_DLOAD_CGI);
            break;

        case CGI_WAIT_ERR:
        case CGI_EXEC_FAIL:
            cur_conn.set_server_err(HTTP_IS_ERR(http_status)
                                        ? http_status
                                        : E_HTTP_INTERNAL_SERVER_ERROR);
            cur_conn.set_state(S_CONN_HANDLE);
            break;
    }
}

void NetworkManager::_conn_dload_cgi(NetConnection& cur_conn) {
    _mod_client_interest_set(cur_conn.socketd(), SCAN_EVENT_OUT);
    if (cur_conn.read_cgi_output(_ibuffer.data, &_ibuffer.size,
                                 _ibuffer.capacity)) {
        cur_conn.set_state(S_CONN_BUILD_HEAD);
        _mod_client_interest_set(cur_conn.socketd(), SCAN_EVENT_IN);
    }
}

void NetworkManager::_conn_dload_body(NetConnection& cur_conn) {
    if (cur_conn.wait_before_next_io()) {
        cur_conn.set_wait_before_next_io(false);
    } else {
        cur_conn.set_wait_before_next_io(true);

        _ibuffer.size =
            recv(cur_conn.socketd(), _ibuffer.data, _ibuffer.capacity, 0);

        if (_ibuffer.size > 0) {
            cur_conn.save_body(_ibuffer.data, &_ibuffer.size);
        } else if (_ibuffer.size == 0) {
            cur_conn.set_state(S_CONN_CLI_CLOSE);
        } else {
            return;
        }
    }
}

void NetworkManager::_conn_build_head(NetConnection& cur_conn) {
    cur_conn.build_headers();
    cur_conn.set_state(S_CONN_SEND_HEAD);
}

void NetworkManager::_conn_send_head(NetConnection& cur_conn) {
    _mod_client_interest_set(cur_conn.socketd(),
                             SCAN_EVENT_IN | SCAN_EVENT_OUT);

    if (cur_conn.wait_before_next_io()) {
        cur_conn.set_wait_before_next_io(false);
    } else {
        switch (_send_headers(cur_conn, cur_conn.socketd())) {
            case SEND_COMPLETE:
                cur_conn.set_state(S_CONN_SEND_BODY);
                cur_conn.set_wait_before_next_io(true);
                break;

            case SEND_UNCOMPLETE:
                _mod_client_interest_set(cur_conn.socketd(),
                                         SCAN_EVENT_IN | SCAN_EVENT_OUT);
                break;

            case SEND_ERROR:
                cur_conn.set_state(S_CONN_CLI_CLOSE);
                break;
        }
    }
}

void NetworkManager::_conn_send_body(NetConnection& cur_conn) {
    if (cur_conn.wait_before_next_io()) {
        cur_conn.set_wait_before_next_io(false);
    } else {
        switch (_send_body(cur_conn, cur_conn.socketd())) {
            case SEND_COMPLETE:
                if (!cur_conn.keep_alive() || cur_conn.kill()) {
                    cur_conn.set_state(S_CONN_CLI_CLOSE);
                    break;
                }
                cur_conn.set_wait_before_next_io(true);

                _mod_client_interest_set(cur_conn.socketd(), SCAN_EVENT_IN);
                cur_conn.reset();

                if (cur_conn.ibuffer_isdata()) {
                    cur_conn.set_state(S_CONN_READ);
                } else {
                    cur_conn.set_state(S_CONN_WAIT);
                }
                break;

            case SEND_UNCOMPLETE:
                _mod_client_interest_set(cur_conn.socketd(),
                                         SCAN_EVENT_IN | SCAN_EVENT_OUT);
                break;

            case SEND_ERROR:
                cur_conn.set_state(S_CONN_CLI_CLOSE);
                break;
        }
    }
}

void NetworkManager::execute() {
    for (http_rconn_list::iterator it = _ready_lst.begin();
         it != _ready_lst.end(); ++it) {
        int sd = *it;
        NetConnection& cur_conn = _connections.find(sd)->second;

        _apply_poll_event_state(cur_conn);

        while (1) {
            cur_conn.reset_state_change();

            switch (cur_conn.state()) {
                case S_CONN_WAIT:
                    _ready_del_lst.push_back(*it);
                    break;

                case S_CONN_READ:
                    _conn_read(cur_conn);
                    break;
                case S_CONN_HANDLE:
                    _conn_handle(cur_conn);
                    break;

                case S_CONN_WRITE_TO_CGI:
                    _conn_write_to_cgi(cur_conn);
                    break;
                case S_CONN_CGI_WAIT:
                    _conn_cgi_wait(cur_conn);
                    break;
                case S_CONN_DLOAD_CGI:
                    _conn_dload_cgi(cur_conn);
                    break;
                case S_CONN_DLOAD_BODY:
                    _conn_dload_body(cur_conn);
                    break;
                case S_CONN_BUILD_HEAD:
                    _conn_build_head(cur_conn);
                    break;

                case S_CONN_SEND_HEAD:
                    _conn_send_head(cur_conn);
                    break;
                case S_CONN_SEND_BODY:
                    _conn_send_body(cur_conn);
                    break;

                case S_CONN_CLI_CLOSE:
                    _close_lst.push_back(*it);
                    break;
            }

            if (cur_conn.state_change() == false)
                break;
        }
    }
    _update_connection_lst();
    SessionManager::get_instance().clean_expired(SESSION_TIMEOUT);
}

/* [OUTBOUND DATA] */

nm_send_state_t NetworkManager::_send_headers(NetConnection& cur_conn,
                                              int socketd) {
    response_status_t status =
        cur_conn.head_stream(_obuffer.data, _obuffer.capacity);

    _obuffer.size = send(socketd, _obuffer.data, status.cur, 0);

    if (ENABLE_IO_HEXDUMP)
        hexdump(_obuffer.data, _obuffer.size);

    if (_obuffer.size >= 0) {
        cur_conn.sync_cursor(_obuffer.size, status.cur);

        if (status.done && static_cast<size_t>(_obuffer.size) == status.cur) {
            return SEND_COMPLETE;
        } else {
            return SEND_UNCOMPLETE;
        }
    } else {
        return SEND_ERROR;
    }
}

nm_send_state_t NetworkManager::_send_body(NetConnection& cur_conn,
                                           int socketd) {
    response_status_t status =
        cur_conn.body_stream(_obuffer.data, _obuffer.capacity);
    _obuffer.size = send(socketd, _obuffer.data, status.cur, 0);

    if (ENABLE_IO_HEXDUMP)
        hexdump(_obuffer.data, _obuffer.size);

    if (_obuffer.size >= 0) {
        cur_conn.sync_cursor(_obuffer.size, status.cur);

        if (status.done && static_cast<size_t>(_obuffer.size) == status.cur) {
            return SEND_COMPLETE;
        } else {
            return SEND_UNCOMPLETE;
        }
    } else {
        return SEND_ERROR;
    }
}

/* [CLOSE CONNECTIONS] */

void NetworkManager::_update_connection_lst() {
    for (http_rconn_list::iterator it = _ready_del_lst.begin();
         it != _ready_del_lst.end(); ++it) {
        _connections.find(*it)->second.set_readiness(false);
        _ready_lst.erase(std::remove(_ready_lst.begin(), _ready_lst.end(), *it),
                         _ready_lst.end());
    }

    for (http_rconn_list::iterator it = _close_lst.begin();
         it != _close_lst.end(); ++it) {
        _ready_lst.erase(std::remove(_ready_lst.begin(), _ready_lst.end(), *it),
                         _ready_lst.end());

        ::close(*it);
        GarbageCollector::get_instance().remove_fd(*it);

        _connections.erase(*it);
    }

    _ready_del_lst.clear();
    _close_lst.clear();
}

/* [SET CONFIG PARAMETERS] */

void NetworkManager::set_recv_segment_siz(size_t size) {
    GarbageCollector::get_instance().remove_ptr(_ibuffer.data);

    delete[] _ibuffer.data;
    _ibuffer.data = new char[size];

    GarbageCollector::get_instance().add_ptr(_ibuffer.data, array_delete);

    _ibuffer.capacity = size;
    _ibuffer.size = 0;
}

void NetworkManager::set_send_segment_siz(size_t size) {
    GarbageCollector::get_instance().remove_ptr(_obuffer.data);

    delete[] _obuffer.data;
    _obuffer.data = new char[size];

    GarbageCollector::get_instance().add_ptr(_obuffer.data, array_delete);

    _obuffer.capacity = size;
    _obuffer.size = 0;
}

void NetworkManager::set_http_max_method_siz(size_t size) {
    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        it->second.set_http_max_method_siz(size);
    }
}

void NetworkManager::set_http_max_uri_siz(size_t size) {
    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        it->second.set_http_max_uri_siz(size);
    }
}

void NetworkManager::set_http_max_header_key_siz(size_t size) {
    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        it->second.set_http_max_header_key_siz(size);
    }
}

void NetworkManager::set_http_max_header_val_siz(size_t size) {
    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        it->second.set_http_max_header_val_siz(size);
    }
}

void NetworkManager::set_http_max_header_amt_siz(size_t size) {
    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        it->second.set_http_max_header_amt_siz(size);
    }
}

void NetworkManager::set_http_max_body_siz(size_t size) {
    for (http_conn_list::iterator it = _connections.begin();
         it != _connections.end(); ++it) {
        it->second.set_http_max_body_siz(size);
    }
}

/* [GENERIC GETTERS] */

size_t NetworkManager::recv_segment_size() const {
    return _ibuffer.capacity;
}

size_t NetworkManager::send_segment_size() const {
    return _obuffer.capacity;
}

size_t NetworkManager::size() const {
    return _connections.size();
}
