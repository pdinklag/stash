#pragma once

namespace stash {
namespace rapl {

struct power {
    double package, core, uncore, dram, psys; // power in µW (watts)

    inline power() : package(0.0), core(0.0), uncore(0.0), dram(0.0), psys(0.0) {
    }

    inline power(double _package, double _core, double _uncore, double _dram, double _psys)
        : package(_package), core(_core), uncore(_uncore), dram(_dram), psys(_psys) {
    }

    inline power(energy de, uint64_t dt) {
        const double ddt = double(dt);
        
        package = (double(de.package) / ddt);
        core    = (double(de.core) / ddt);
        uncore  = (double(de.uncore) / ddt);
        dram    = (double(de.dram) / ddt);
        psys    = (double(de.psys) / ddt);
    }

    inline power operator+(const power& other) {
        return power(
            package + other.package,
            core    + other.core,
            uncore  + other.uncore,
            dram    + other.dram,
            psys    + other.psys
        );
    }

    inline power& operator+=(const power& other) {
        package += other.package;
        core    += other.core;
        uncore  += other.uncore;
        dram    += other.dram;
        psys    += other.psys;
        return *this;
    }

    inline power operator-(const power& other) {
        return power (
            package - other.package,
            core    - other.core,
            uncore  - other.uncore,
            dram    - other.dram,
            psys    - other.psys
        );
    }

    inline power operator/(double d) {
        return power(
            package / d,
            core    / d,
            uncore  / d,
            dram    / d,
            psys    / d
        );
    }
};

}}

#include <ostream>

std::ostream& operator<<(std::ostream& os, const stash::rapl::power& p) {
    os << "(" << p.package << "," << p.core << "," << p.uncore << "," << p.dram << "," << p.psys << ")";
    return os;
}
