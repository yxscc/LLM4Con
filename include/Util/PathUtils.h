#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>
#include <filesystem>

namespace PathUtils {

inline std::string extractBaseFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

inline bool arePathsLikelySameFile(const std::string& path1, const std::string& path2) {
    std::filesystem::path p1 = std::filesystem::path(path1).lexically_normal();
    std::filesystem::path p2 = std::filesystem::path(path2).lexically_normal();

    if (p1.filename() != p2.filename()) return false;

    auto it1 = p1.end();
    auto it2 = p2.end();
    while (it1 != p1.begin() && it2 != p2.begin()) {
        --it1;
        --it2;
        if (*it1 != *it2) return false;
    }
    return true;
}

} // namespace PathUtils

#endif // PATH_UTILS_H
