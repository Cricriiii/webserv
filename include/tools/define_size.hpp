#ifndef TO_SIZE_T_HPP
#define TO_SIZE_T_HPP

#include <cstddef>
#include <limits>

template <typename DEST_TYP, typename ORIG_TYP>
static inline DEST_TYP define_size(DEST_TYP default_siz,
                                   ORIG_TYP proposed_siz) {
    if (proposed_siz < 0) {
        return default_siz;
    }
    if (static_cast<double>(proposed_siz) >
        static_cast<double>(std::numeric_limits<DEST_TYP>::max())) {
        return default_siz;
    } else {
        return static_cast<DEST_TYP>(proposed_siz);
    }
}

#endif