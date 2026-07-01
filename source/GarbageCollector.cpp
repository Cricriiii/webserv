#include "GarbageCollector.hpp"

#include <stdlib.h>
#include <unistd.h>

// Implémentation des deleters par défaut
void default_delete(void* p) {
    ::operator delete(p);
}
void array_delete(void* p) {
    ::operator delete[](p);
}
void free_delete(void* p) {
    free(p);
}

GarbageCollector& GarbageCollector::get_instance() {
    static GarbageCollector instance;
    return instance;
}

GarbageCollector::GarbageCollector() {
}
GarbageCollector::~GarbageCollector() {
}

void GarbageCollector::add_ptr(void* ptr, deleter_t deleter) {
    if (ptr && deleter) {
        ManagedPtr item;
        item.ptr = ptr;
        item.deleter = deleter;
        _ptrs.push_back(item);
    }
}

void GarbageCollector::add_fd(int fd) {
    if (fd >= 0) {
        _fds.push_back(fd);
    }
}

void GarbageCollector::remove_ptr(void* ptr) {
    for (std::vector<ManagedPtr>::iterator it = _ptrs.begin();
         it != _ptrs.end(); ++it) {
        if (it->ptr == ptr) {
            _ptrs.erase(it);
            return;
        }
    }
}

void GarbageCollector::remove_fd(int fd) {
    for (std::vector<int>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
        if (*it == fd) {
            _fds.erase(it);
            return;
        }
    }
}

void GarbageCollector::clean_all() {
    for (size_t i = 0; i < _ptrs.size(); ++i) {
        if (_ptrs[i].ptr) {
            _ptrs[i].deleter(_ptrs[i].ptr);
        }
    }
    _ptrs.clear();

    for (size_t i = 0; i < _fds.size(); ++i) {
        if (_fds[i] >= 0)
            close(_fds[i]);
    }
    _fds.clear();
}
