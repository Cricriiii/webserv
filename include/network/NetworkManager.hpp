#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

/* [PREPROCESSOR
 */

/* --------------[os
 * dependant]-----------------------------------------------------------------
 */

#include "os.hpp"

#if defined(LINUX)
#include <sys/epoll.h>
#define MAX_EVENTS 256
typedef struct epoll_event event_list;

#elif defined(APPLE)
#include <sys/event.h>
#include <sys/time.h>
#define MAX_EVENTS 256
typedef struct kevent event_list;

#endif

/* --------------[os
 * dependant]-----------------------------------------------------------------
 */

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "GarbageCollector.hpp"
#include "conf.hpp"
#include "network/Acceptor.hpp"
#include "network/DataTypes.hpp"
#include "network/NetConnection.hpp"
#include "network/Router.hpp"
#include "network/SessionManager.hpp"
#include "vserver/vserver_creator.hpp"

/* [ENUMS]
 */

typedef enum send_status {
    SEND_COMPLETE = 1,
    SEND_UNCOMPLETE = 2,
    SEND_ERROR = 3
} nm_send_state_t;

/* [IMPLEMENTATION]
 */

class NetworkManager {
    /* --------------[typedef]----------------------------------------------------------------------
     */
    typedef std::map<int, NetConnection> http_conn_list;
    typedef std::vector<int> http_rconn_list;
    typedef std::set<int> http_lport_list;
    typedef std::map<int, Acceptor*> acceptor_list;
    typedef std::vector<VServer> vserver_list;

    /* --------------[constructors]-----------------------------------------------------------------
     */
public:
    NetworkManager();
    NetworkManager(vserver_list& vserv, const http_lport_list& lports,
                   const conf_http_t& http_conf);
    ~NetworkManager();

private:
    NetworkManager(const NetworkManager& other);
    NetworkManager& operator=(const NetworkManager& other);

    /* --------------[private
     * variables]------------------------------------------------------------ */

private:
    const conf_http_t& _http_conf;

    int _event_fd;
    acceptor_list _acceptors;
    Router _router;

    http_conn_list _connections;

    http_rconn_list _ready_lst;
    http_rconn_list _ready_del_lst;
    http_rconn_list _close_lst;

    event_list _events[MAX_EVENTS];

    struct buffer _ibuffer;
    struct buffer _obuffer;

    /* --------------[reactor - core
     * pattern]------------------------------------------------------- */
public:
    void listen();
    void execute();

private:
    void _conn_read(NetConnection& cur_conn);
    void _conn_handle(NetConnection& cur_conn);
    void _conn_write_to_cgi(NetConnection& cur_conn);
    void _conn_cgi_wait(NetConnection& cur_conn);
    void _conn_dload_cgi(NetConnection& cur_conn);
    void _conn_dload_body(NetConnection& cur_conn);
    void _conn_build_head(NetConnection& cur_conn);
    void _conn_send_head(NetConnection& cur_conn);
    void _conn_send_body(NetConnection& cur_conn);

    bool _cgi_kill_if_timeout(NetConnection& cur_conn, long long timeout_time);

    /* --------------[connection
     * management]-------------------------------------------------------- */
private:
    void _new_acceptor(int port, protocol_type proto);
    void _init_event_polling();
    void _mod_client_interest_set(int socketd, poll_op_t new_state);
    void _update_connection_lst();

    /* --------------[outbound
     * data]---------------------------------------------------------------- */
private:
    nm_send_state_t _send_headers(NetConnection& cur_conn, int socketd);
    nm_send_state_t _send_body(NetConnection& cur_conn, int socketd);

    /* --------------[config
     * parameters]------------------------------------------------------------
     */
public:
    void set_recv_segment_siz(size_t size);
    void set_send_segment_siz(size_t size);
    void set_http_max_method_siz(size_t size);
    void set_http_max_uri_siz(size_t size);
    void set_http_max_header_key_siz(size_t size);
    void set_http_max_header_val_siz(size_t size);
    void set_http_max_header_amt_siz(size_t size);
    void set_http_max_body_siz(size_t size);

    /* --------------[generic
     * getters]-------------------------------------------------------------- */
public:
    size_t recv_segment_size() const;
    size_t send_segment_size() const;
    size_t size() const;

    /* --------------[default
     * values]--------------------------------------------------------------- */

private:
    static const size_t _DFLT_RECV_SEGMENT_SIZ = 16384;
    static const size_t _DFLT_SEND_SEGMENT_SIZ = 16384;
};

#endif
