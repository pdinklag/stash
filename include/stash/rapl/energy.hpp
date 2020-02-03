#pragma once

namespace stash {
namespace rapl {

struct energy {
    uint64_t package, core, uncore, dram, psys; // energy in µJ (microjoules)

    inline energy() : package(0), core(0), uncore(0), dram(0), psys(0) {
    }

    inline energy(uint64_t _package, uint64_t _core, uint64_t _uncore, uint64_t _dram, uint64_t _psys)
        : package(_package), core(_core), uncore(_uncore), dram(_dram), psys(_psys) {
    }

    inline energy operator-(const energy& other) {
        return energy (
            package - other.package,
            core    - other.core,
            uncore  - other.uncore,
            dram    - other.dram,
            psys    - other.psys
        );
    }
};

}}

#include <ostream>

std::ostream& operator<<(std::ostream& os, const stash::rapl::energy& e) {
    os << "(" << e.package << "," << e.core << "," << e.uncore << "," << e.dram << "," << e.psys << ")";
    return os;
}
