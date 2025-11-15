// backend/src/Util.hpp
#pragma once
#include <string>
#include <vector>

inline bool arg_has(const std::vector<std::string>& a, const std::string& k) {
    for (size_t i=0;i+0<a.size();++i) if (a[i]==k) return true;
    return false;
}
inline std::string arg_val(const std::vector<std::string>& a, const std::string& k, const std::string& def) {
    for (size_t i=0;i+1<a.size();++i) if (a[i]==k) return a[i+1];
    return def;
}
