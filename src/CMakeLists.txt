add_executable(rapl-test rapl_test.cpp)

target_include_directories(rapl-test PUBLIC ${POWERCAP_INCLUDE_DIRS} ${TLX_INCLUDE_DIRS})
target_link_libraries(rapl-test ${POWERCAP_LIBRARIES} ${TLX_LIBRARIES})
