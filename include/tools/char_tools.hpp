#ifndef CHAR_TOOLS_HPP
#define CHAR_TOOLS_HPP

#include <algorithm>
#include <map>
#include <sstream>
#include <string>

namespace char_tools {

template <typename LOOK_FOR>
inline bool is_in_string(const std::string& find_in, const LOOK_FOR& look_for) {
    return find_in.find(look_for, 0) != std::string::npos;
}

template <typename CONTAINER, typename CMP_FN_T, typename CMP_FN_A>
inline const typename CONTAINER::const_iterator find_if(
    const CONTAINER& c, CMP_FN_T cmp_fn(CMP_FN_A)) {
    return std::find_if(c.begin(), c.end(),
                        std::ptr_fun<CMP_FN_A, CMP_FN_T>(cmp_fn));
}

template <typename CONTAINER, typename CMP_FN_T, typename CMP_FN_A>
inline typename CONTAINER::iterator find_if(CONTAINER& c,
                                            CMP_FN_T cmp_fn(CMP_FN_A)) {
    return std::find_if(c.begin(), c.end(),
                        std::ptr_fun<CMP_FN_A, CMP_FN_T>(cmp_fn));
}

template <typename CONTAINER, typename CMP_FN_T, typename CMP_FN_A>
inline int all_of(const CONTAINER& c, CMP_FN_T cmp_fn(CMP_FN_A)) {
    return std::find_if(c.begin(), c.end(),
                        std::not1(std::ptr_fun<CMP_FN_A, CMP_FN_T>(cmp_fn))) ==
           c.end();
}

template <typename CONTAINER, typename CMP_FN_T, typename CMP_FN_A>
inline int none_of(const CONTAINER& c, CMP_FN_T cmp_fn(CMP_FN_A)) {
    return std::find_if(c.begin(), c.end(),
                        std::ptr_fun<CMP_FN_A, CMP_FN_T>(cmp_fn)) == c.end();
}

template <typename CMP_FN_T, typename CMP_FN_A>
bool string_cmp(const std::string& compared, const std::string& base,
                CMP_FN_T cmp_fn(CMP_FN_A)) {
    std::string t_compared(compared), t_base(base);
    std::transform(t_compared.begin(), t_compared.end(), t_compared.begin(),
                   std::ptr_fun<CMP_FN_A, CMP_FN_T>(cmp_fn));
    std::transform(t_base.begin(), t_base.end(), t_base.begin(),
                   std::ptr_fun<CMP_FN_A, CMP_FN_T>(cmp_fn));
    return t_compared == t_base;
}

template <typename M_MAP>
typename M_MAP::const_iterator map_string_find_nocase(const M_MAP& map,
                                                      const std::string& base) {
    typedef typename M_MAP::const_iterator const_it;

    const_it cit = map.begin();
    const_it cite = map.end();

    for (; cit != cite; ++cit) {
        if (string_cmp(cit->first, base, ::tolower)) {
            return cit;
        }
    }
    return map.end();
}

template <typename M_MAP>
typename M_MAP::iterator map_string_find_nocase(M_MAP& map,
                                                const std::string& base) {
    typename M_MAP::const_iterator cit =
        map_string_find_nocase(static_cast<const M_MAP&>(map), base);

    if (cit == map.end())
        return map.end();

    typename M_MAP::iterator it = map.begin();
    std::advance(
        it, std::distance<typename M_MAP::const_iterator>(map.begin(), cit));
    return it;
}

std::string itostr(int nb);

void trim_string(std::string& string, char trimmed);
}  // namespace char_tools

#endif
