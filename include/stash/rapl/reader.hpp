#pragma once

#include <stash/rapl/energy.hpp>
#include <stash/rapl/rapl_error.hpp>

#include <powercap/powercap-rapl.h>

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

private:
    powercap_rapl_pkg m_pkg;
    feature_support m_support;

public:
    static inline uint32_t num_packages() {
        return powercap_rapl_get_num_packages();
    }

    inline reader(uint32_t package = 0) {
        if(num_packages() < package) {
            throw rapl_error("package not available");
        }

        powercap_rapl_init(package, &m_pkg, true);

        m_support.package = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_PACKAGE);
        m_support.core    = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_CORE);
        m_support.uncore  = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_UNCORE);
        m_support.dram    = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_DRAM);
        m_support.psys    = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_PSYS);
    }

    inline energy read() const {
        energy e;
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_PACKAGE, &e.package);
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_CORE, &e.core);
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_UNCORE, &e.uncore);
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_DRAM, &e.dram);
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_PSYS, &e.psys);
        return e;
    }

    inline ~reader() {
        powercap_rapl_destroy(&m_pkg);
    }

    const feature_support& support = m_support;
};

}}
