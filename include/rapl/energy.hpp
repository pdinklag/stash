#pragma once

namespace rapl {
    
struct energy {
    uint64_t core, uncore, dram; // energy in µJ (microjoules)

    inline energy() : core(0), uncore(0), dram(0) {
    }

    inline energy(uint64_t _core, uint64_t _uncore, uint64_t _dram)
        : core(_core), uncore(_uncore), dram(_dram) {
    }

    inline uint64_t total() const {
        return core + uncore + dram;
    }

    inline energy operator-(const energy& other) {
        return energy {
            core   - other.core,
            uncore - other.uncore,
            dram   - other.dram
        };
    }
};

}

#include <ostream>

std::ostream& operator<<(std::ostream& os, const rapl::energy& e) {
    os << "(" << e.core << "," << e.uncore << "," << e.dram << ")";
    return os;
}
