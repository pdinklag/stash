#pragma once

#include <string>
#include <sys/stat.h>

namespace stash {

inline size_t file_size(const std::string& filename) {
    struct stat buf;
    stat(filename.c_str(), &buf);
    return buf.st_size;
}

}
