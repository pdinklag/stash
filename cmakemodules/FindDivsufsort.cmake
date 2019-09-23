# Find DIVSUFSORT
include(FindPackageHandleStandardArgs)

set(DIVSUFSORT_ROOT_DIR "" CACHE PATH "Path to DIVSUFSORT")

find_path(DIVSUFSORT_INCLUDE_DIR divsufsort64.h PATHS ${DIVSUFSORT_ROOT_DIR})
find_library(DIVSUFSORT_LIBRARY divsufsort64 PATHS ${DIVSUFSORT_ROOT_DIR})
find_package_handle_standard_args(DIVSUFSORT DEFAULT_MSG DIVSUFSORT_INCLUDE_DIR DIVSUFSORT_LIBRARY)

if(DIVSUFSORT_FOUND)
    set(DIVSUFSORT_INCLUDE_DIRS ${DIVSUFSORT_INCLUDE_DIR})
    set(DIVSUFSORT_LIBRARIES ${DIVSUFSORT_LIBRARY})
    include_directories(${DIVSUFSORT_INCLUDE_DIRS})
endif()