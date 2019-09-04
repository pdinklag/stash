add_executable(rapl-test rapl_test.cpp)

target_include_directories(rapl-test PUBLIC ${POWERCAP_INCLUDE_DIRS} ${TLX_INCLUDE_DIRS})
target_link_libraries(rapl-test ${POWERCAP_LIBRARIES} ${TLX_LIBRARIES})

add_executable(coding coding.cpp)

target_include_directories(coding PUBLIC ${TLX_INCLUDE_DIRS})
target_link_libraries(coding ${TLX_LIBRARIES})
