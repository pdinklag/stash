# Find cmph
include(FindPackageHandleStandardArgs)

find_path(CMPH_INCLUDE_DIR cmph.h)
find_library(CMPH_LIBRARY cmph)
find_package_handle_standard_args(CMPH DEFAULT_MSG CMPH_INCLUDE_DIR CMPH_LIBRARY)

if(CMPH_FOUND)
    set(CMPH_INCLUDE_DIRS ${CMPH_INCLUDE_DIR})
    set(CMPH_LIBRARIES ${CMPH_LIBRARY})
endif()
