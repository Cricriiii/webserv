#include "vserver/vserver.hpp"

#include "conf.hpp"
#include "http/HTTP_Errors.hpp"
#include "http/HTTP_Request.hpp"
#include "tools/char_tools.hpp"

VServer::VServer(const vserver_t& server) : _vserver(server) {
}

VServer::~VServer() {
}

const request_context_t VServer::execute(const HTTP_Request& request) {
    clear_context();

    _context.request = const_cast<HTTP_Request*>(&request);
    construct_request_path();

    const location_t* location = find_location();
    build_context(location);

    verif_body_size();
    return _context;
}

void VServer::construct_request_path() {
    size_t pos = _context.request->start_line.URL.find('?');
    if (pos != std::string::npos)
        _context.query = _context.request->start_line.URL.substr(pos + 1);
}

const location_t* VServer::find_location() const {
    std::string path = _context.request->start_line.URL;

    while (true) {
        for (size_t i = 0; i < _vserver.location.size(); ++i) {
            if (path == _vserver.location[i].path)
                return &_vserver.location[i];
        }

        if (path == "/")
            break;

        if (path.size() && path[path.size() - 1] == '/')
            path.erase(path.size() - 1);
        size_t pos = path.find_last_of('/');
        if (pos == 0)
            path = "/";
        else if (pos == std::string::npos)
            break;
        else
            path = path.substr(0, pos + 1);
    }
    return NULL;
}

void VServer::build_context(const location_t* location) {
    for (std::map<int, std::string>::const_iterator it =
             _vserver.errors.begin();
         it != _vserver.errors.end(); ++it) {
        _context.errors.insert(
            std::make_pair(it->first, _vserver.root + it->second));
    }
    _context.server_name = _vserver.name;
    _context.server_port = char_tools::itostr(_vserver.port);

    if (!location) {
        _context.resolved_path =
            _vserver.root + _context.request->start_line.URL.substr(
                                0, _context.request->start_line.URL.find('?'));
        _context.index = _vserver.index;
        _context.autoindex = false;
    } else {
        if (location->root.empty()) {
            _context.resolved_path =
                _vserver.root +
                _context.request->start_line.URL.substr(
                    0, _context.request->start_line.URL.find('?'));
            _context.root = _vserver.root;
        } else {
            _context.resolved_path =
                location->root +
                _context.request->start_line.URL.substr(
                    0, _context.request->start_line.URL.find('?'));
            _context.root = location->root;
        }

        if (location->index.empty())
            _context.index = _vserver.index;
        else
            _context.index = location->index;

        _context.limit_except = location->limit_except;
        _context.autoindex = location->autoindex;

        _context.cgi = location->cgi;
        _context.cgi_extension = location->cgi_extension;
        _context.cgi_path = location->cgi_path;
    }

    if (_context.resolved_path[_context.resolved_path.size() - 1] == '/')
        _context.directory = true;
    else
        _context.directory = false;
}

void VServer::verif_body_size() const {
    if (_context.request->entity_body.size >
        static_cast<size_t>(_vserver.max_body_size)) {
        _context.request->status = E_HTTP_CONTENT_TOO_LARGE;
    }
}

void VServer::clear_context() {
    _context = request_context_t();
}

bool VServer::is_it_me(const std::string& name, int port) const {
    for (std::set<std::string>::const_iterator it = _vserver.name.begin();
         it != _vserver.name.end(); ++it) {
        if (*it == name && _vserver.port == port)
            return true;
    }
    return false;
}

bool VServer::is_port(int port) const {
    return (_vserver.port == port);
}

void VServer::print_context() const {
    std::cout << "\n ----- Context ----- \n" << std::endl;

    std::cout << "query: " << _context.query << std::endl;
    std::cout << "resolved_path: " << _context.resolved_path << std::endl;
    std::cout << "directory: " << _context.directory << std::endl;
    std::cout << "autoindex: " << _context.autoindex << std::endl;
    std::cout << "cgi_extension: " << _context.cgi_extension << std::endl;
    std::cout << "cgi_path: " << _context.cgi_path << std::endl;
    for (std::map<int, std::string>::const_iterator it =
             _context.errors.begin();
         it != _context.errors.end(); ++it)
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    std::cout << "limit_except: ";
    for (std::set<std::string>::const_iterator it =
             _context.limit_except.begin();
         it != _context.limit_except.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
    std::cout << "file_succes_download: " << _context.file_succes_download
              << std::endl;
    std::cout << "file_succes_delete: " << _context.file_succes_delete
              << std::endl;
    std::cout << "file_tmp_css: " << _context.file_tmp_css << std::endl;
    std::cout << "server_name: ";
    for (std::set<std::string>::const_iterator it =
             _context.server_name.begin();
         it != _context.server_name.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
    std::cout << "server_port: " << _context.server_port << std::endl;
    std::cout << "root: " << _context.root << std::endl;
    std::cout << "index: ";
    for (std::vector<std::string>::const_iterator it = _context.index.begin();
         it != _context.index.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
}
