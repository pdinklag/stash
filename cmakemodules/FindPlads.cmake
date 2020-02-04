# Find PLADS
include(FindPackageHandleStandardArgs)

set(PLADS_ROOT_DIR "" CACHE PATH "Path to PlaDS")

find_path(PLADS_INCLUDE_DIR plads/bit_vector/bit_vector.hpp PATHS ${PLADS_ROOT_DIR} PATH_SUFFIXES include)
find_library(PLADS_LIBRARY plads PATHS ${PLADS_ROOT_DIR} PATH_SUFFIXES build)
find_package_handle_standard_args(PLADS DEFAULT_MSG PLADS_INCLUDE_DIR PLADS_LIBRARY)

if(PLADS_FOUND)
    add_definitions(-DPLADS_FOUND)
    set(PLADS_INCLUDE_DIRS ${PLADS_INCLUDE_DIR})
    set(PLADS_LIBRARIES ${PLADS_LIBRARY})
    include_directories(${PLADS_INCLUDE_DIRS})
endif()
