# Find Powercap
include(FindPackageHandleStandardArgs)

find_path(POWERCAP_INCLUDE_DIR powercap/powercap.h)
find_library(POWERCAP_LIBRARY powercap)
find_package_handle_standard_args(POWERCAP DEFAULT_MSG POWERCAP_INCLUDE_DIR POWERCAP_LIBRARY)

if(POWERCAP_FOUND)
    add_definitions(-DPOWERCAP_FOUND)
    set(POWERCAP_INCLUDE_DIRS ${POWERCAP_INCLUDE_DIR})
    set(POWERCAP_LIBRARIES ${POWERCAP_LIBRARY})
endif()
