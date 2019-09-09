#pragma once

namespace rapl {

struct Power {
    double core, uncore, dram; // power in W (watts)

    inline Power() : core(0.0), uncore(0.0), dram(0.0) {
    }

    inline Power(double _core, double _uncore, double _dram)
        : core(_core), uncore(_uncore), dram(_dram) {
    }

    inline Power(Energy de, uint64_t dt) {
        const double ddt = double(dt);
        
        core =   (double(de.core) / ddt) / 1000.0;
        uncore = (double(de.uncore) / ddt) / 1000.0;
        dram =   (double(de.dram) / ddt) / 1000.0;
    }

    inline double total() const {
        return core + uncore + dram;
    }

    inline Power operator+(const Power& other) {
        return Power {
            core   + other.core,
            uncore + other.uncore,
            dram   + other.dram
        };
    }

    inline Power& operator+=(const Power& other) {
        core   += other.core;
        uncore += other.uncore;
        dram   += other.dram;
        return *this;
    }

    inline Power operator-(const Power& other) {
        return Power {
            core   - other.core,
            uncore - other.uncore,
            dram   - other.dram
        };
    }

    inline Power operator/(double d) {
        return Power {
            core   / d,
            uncore / d,
            dram   / d
        };
    }
};

}

#include <ostream>

std::ostream& operator<<(std::ostream& os, const rapl::Power& p) {
    os << "(" << p.core << "," << p.uncore << "," << p.dram << ")";
    return os;
}
