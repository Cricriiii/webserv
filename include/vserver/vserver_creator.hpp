#ifndef VSERVER_CREATOR_HPP
#define VSERVER_CREATOR_HPP

#include <vector>

#include "conf.hpp"
#include "vserver/vserver.hpp"

class VServerCreator {
private:
    VServerCreator();
    ~VServerCreator();

public:
    static std::vector<VServer> create_vservers(
        const std::vector<vserver_t>& confs);
};

#endif
