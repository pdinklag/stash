#pragma once

namespace rapl {

struct power {
    double core, uncore, dram; // power in W (watts)

    inline power() : core(0.0), uncore(0.0), dram(0.0) {
    }

    inline power(double _core, double _uncore, double _dram)
        : core(_core), uncore(_uncore), dram(_dram) {
    }

    inline power(energy de, uint64_t dt) {
        const double ddt = double(dt);
        
        core =   (double(de.core) / ddt) / 1000.0;
        uncore = (double(de.uncore) / ddt) / 1000.0;
        dram =   (double(de.dram) / ddt) / 1000.0;
    }

    inline double total() const {
        return core + uncore + dram;
    }

    inline power operator+(const power& other) {
        return power {
            core   + other.core,
            uncore + other.uncore,
            dram   + other.dram
        };
    }

    inline power& operator+=(const power& other) {
        core   += other.core;
        uncore += other.uncore;
        dram   += other.dram;
        return *this;
    }

    inline power operator-(const power& other) {
        return power {
            core   - other.core,
            uncore - other.uncore,
            dram   - other.dram
        };
    }

    inline power operator/(double d) {
        return power {
            core   / d,
            uncore / d,
            dram   / d
        };
    }
};

}

#include <ostream>

std::ostream& operator<<(std::ostream& os, const rapl::power& p) {
    os << "(" << p.core << "," << p.uncore << "," << p.dram << ")";
    return os;
}
