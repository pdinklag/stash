#pragma once

#include <rapl/energy.hpp>
#include <rapl/rapl_error.hpp>

#include <powercap/powercap-rapl.h>

namespace rapl {

class reader {
private:
    powercap_rapl_pkg m_pkg;
    bool m_support_core, m_support_uncore, m_support_dram;

public:
    inline reader(uint32_t package = 0) {
        const auto npackages = powercap_rapl_get_num_packages();
        if(npackages < package) {
            throw rapl_error("package not available");
        }

        powercap_rapl_init(package, &m_pkg, true);
        m_support_core   = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_CORE);
        m_support_uncore = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_UNCORE);
        m_support_dram   = powercap_rapl_is_zone_supported(&m_pkg, POWERCAP_RAPL_ZONE_DRAM);
    }

    inline energy read() const {
        // TODO: what happens if certain zones are not supported?
        energy e;
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_CORE, &e.core);
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_UNCORE, &e.uncore);
        powercap_rapl_get_energy_uj(&m_pkg, POWERCAP_RAPL_ZONE_DRAM, &e.dram);
        return e;
    }

    inline ~reader() {
        powercap_rapl_destroy(&m_pkg);
    }

    const bool& support_core   = m_support_core;
    const bool& support_uncore = m_support_uncore;
    const bool& support_dram   = m_support_dram;
};

}
