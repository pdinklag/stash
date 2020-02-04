# Find PCM
include(FindPackageHandleStandardArgs)

find_path(PCM_INCLUDE_DIR pcm/cpucounters.h)
find_library(PCM_LIBRARY pcm)
find_package_handle_standard_args(PCM DEFAULT_MSG PCM_INCLUDE_DIR PCM_LIBRARY)

if(PCM_FOUND)
    add_definitions(-DPCM_FOUND)
    set(PCM_INCLUDE_DIRS ${PCM_INCLUDE_DIR})
    set(PCM_LIBRARIES ${PCM_LIBRARY})
endif()
