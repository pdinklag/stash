#pragma once

#include <stdexcept>

class rapl_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
