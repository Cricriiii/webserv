#ifndef GARBAGECOLLECTOR_HPP
#define GARBAGECOLLECTOR_HPP

#include <vector>

class GarbageCollector {
public:
    static GarbageCollector& get_instance();

    typedef void (*deleter_t)(void*);

    void add_ptr(void* ptr, deleter_t deleter);
    void add_fd(int fd);

    void remove_ptr(void* ptr);
    void remove_fd(int fd);

    void clean_all();

private:
    struct ManagedPtr {
        void* ptr;
        deleter_t deleter;
    };

    GarbageCollector();
    ~GarbageCollector();
    GarbageCollector(const GarbageCollector&);
    GarbageCollector& operator=(const GarbageCollector&);

    std::vector<ManagedPtr> _ptrs;
    std::vector<int> _fds;
};

void default_delete(void* p);
void array_delete(void* p);
void free_delete(void* p);

#endif