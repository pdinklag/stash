#pragma once

#include <cstdint>

namespace stash {
namespace rapl {

#ifdef MAX_RAPL_PACKAGES
static constexpr uint32_t max_rapl_packages = MAX_RAPL_PACKAGES;
#else
static constexpr uint32_t max_rapl_packages = 2;
#endif

}}
