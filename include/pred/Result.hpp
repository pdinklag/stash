#pragma once

namespace pred {

template<typename item_t>
struct Result {
    bool   exists;
    bool   contained;
    item_t value;

    inline operator bool() const {
        return exists;
    }

    inline operator item_t() const {
        return value;
    }
};

}
