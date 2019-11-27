# rapl-test
add_executable(rapl-test rapl_test.cpp)

target_include_directories(rapl-test PUBLIC ${POWERCAP_INCLUDE_DIRS} ${TLX_INCLUDE_DIRS})
target_link_libraries(rapl-test ${POWERCAP_LIBRARIES} ${TLX_LIBRARIES})

# coding
add_executable(coding coding.cpp)

target_include_directories(coding PUBLIC ${TLX_INCLUDE_DIRS})
target_link_libraries(coding ${TLX_LIBRARIES})

# bwt
add_executable(bwt bwt.cpp)

target_include_directories(bwt PUBLIC ${TLX_INCLUDE_DIRS} ${DIVSUFSORT_INCLUDE_DIRS})
target_link_libraries(bwt ${TLX_LIBRARIES} ${DIVSUFSORT_LIBRARIES})

# interface benchmark
add_executable(interface interface.cpp)

target_include_directories(interface PUBLIC ${TLX_INCLUDE_DIRS} ${DIVSUFSORT_INCLUDE_DIRS})
target_link_libraries(interface ${TLX_LIBRARIES} ${DIVSUFSORT_LIBRARIES})

# runs
add_executable(runs runs.cpp)

target_include_directories(runs PUBLIC ${TLX_INCLUDE_DIRS})
target_link_libraries(runs ${TLX_LIBRARIES})

# pred
add_executable(pred pred.cpp malloc.cpp)

target_include_directories(pred PUBLIC ${TLX_INCLUDE_DIRS})
target_link_libraries(pred ${TLX_LIBRARIES})

# sandbox
add_executable(sandbox sandbox.cpp)

target_include_directories(sandbox PUBLIC ${TLX_INCLUDE_DIRS})
target_link_libraries(sandbox ${TLX_LIBRARIES})
