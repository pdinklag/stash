# Find TLX
include(FindPackageHandleStandardArgs)

set(TLX_ROOT_DIR "" CACHE PATH "Path to TLX")

find_path(TLX_INCLUDE_DIR tlx/define.hpp PATHS ${TLX_ROOT_DIR})
find_library(TLX_LIBRARY tlx PATHS ${TLX_ROOT_DIR} PATH_SUFFIXES build/tlx)
find_package_handle_standard_args(TLX DEFAULT_MSG TLX_INCLUDE_DIR TLX_LIBRARY)

if(TLX_FOUND)
    set(TLX_INCLUDE_DIRS ${TLX_INCLUDE_DIR})
    set(TLX_LIBRARIES ${TLX_LIBRARY})
    include_directories(${TLX_INCLUDE_DIRS})
endif()
