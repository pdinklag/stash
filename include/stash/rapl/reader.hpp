#pragma once

#include <array>
#include <iostream>

#include <stash/rapl/config.hpp>
#include <stash/rapl/energy_buffer.hpp>

#ifdef POWERCAP_FOUND

#include <powercap/powercap-rapl.h>
#define RAPL

namespace stash {
namespace rapl {

class reader {
public:
    struct feature_support {
        bool package : 1;
        bool core    : 1;
        bool uncore  : 1;
        bool dram    : 1;
        bool psys    : 1;
    };

    static inline uint32_t num_packages() {
        return powercap_rapl_get_num_packages();
    }

private:
    uint32_t m_num_packages;
    std::array<powercap_rapl_pkg, max_rapl_packages> m_pkg;

public:
    inline reader() {
        m_num_packages = num_packages();
        
        if(max_rapl_packages < m_num_packages) {
            std::cerr
                << "WARNING: There are more RAPL packages available (" << m_num_packages
                << ") than configured by the MAX_RAPL_PACKAGES macro (" << max_rapl_packages
                << "). RAPL read results may be incomplete!"
                << std::endl;

            m_num_packages = max_rapl_packages;
        }

        for(uint32_t i = 0; i < m_num_packages; i++) {
            powercap_rapl_init(i, &m_pkg[i], true);
        }
    }

    inline feature_support support(uint32_t package) const {
        feature_support s;
        s.package = powercap_rapl_is_zone_supported(&m_pkg[package], POWERCAP_RAPL_ZONE_PACKAGE);
        s.core    = powercap_rapl_is_zone_supported(&m_pkg[package], POWERCAP_RAPL_ZONE_CORE);
        s.uncore  = powercap_rapl_is_zone_supported(&m_pkg[package], POWERCAP_RAPL_ZONE_UNCORE);
        s.dram    = powercap_rapl_is_zone_supported(&m_pkg[package], POWERCAP_RAPL_ZONE_DRAM);
        s.psys    = powercap_rapl_is_zone_supported(&m_pkg[package], POWERCAP_RAPL_ZONE_PSYS);
        return s;
    }

    inline feature_support support() const {
        feature_support s = support(0);
        feature_support spkg;
        for(uint32_t i = 1; i < m_num_packages; i++) {
            spkg = support(i);
            s.package = s.package && spkg.package;
            s.core    = s.core    && spkg.core;
            s.uncore  = s.uncore  && spkg.uncore;
            s.dram    = s.dram    && spkg.dram;
            s.psys    = s.psys    && spkg.psys;
        }
        return s;
    }

    inline energy read(uint32_t package) const {
        energy e;
        powercap_rapl_get_energy_uj(&m_pkg[package], POWERCAP_RAPL_ZONE_PACKAGE, &e.package);
        powercap_rapl_get_energy_uj(&m_pkg[package], POWERCAP_RAPL_ZONE_CORE,    &e.core);
        powercap_rapl_get_energy_uj(&m_pkg[package], POWERCAP_RAPL_ZONE_UNCORE,  &e.uncore);
        powercap_rapl_get_energy_uj(&m_pkg[package], POWERCAP_RAPL_ZONE_DRAM,    &e.dram);
        powercap_rapl_get_energy_uj(&m_pkg[package], POWERCAP_RAPL_ZONE_PSYS,    &e.psys);
        return e;
    }

    inline energy_buffer read() const {
        energy_buffer buf;
        for(uint32_t i = 0; i < m_num_packages; i++) {
            buf[i] = read(i);
        }
        return buf;
    }

    inline ~reader() {
        for(uint32_t i = 0; i < m_num_packages; i++) {
            powercap_rapl_destroy(&m_pkg[i]);
        }
    }
};

}}

#else
#undef RAPL
#pragma message("the RAPL reader requires powercap")
#endif
