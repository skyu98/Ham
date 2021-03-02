set(base_SRCS
  CountDownLatch.cc
  # CurrentThread.cc
  Exception.cc
  ThreadPool.cc
  Timestamp.cc
  Log.cc
  )

add_library(ham_base ${base_SRCS})
target_link_libraries(ham_base ${GTEST_BOTH_LIBRARIES} pthread rt)

install(TARGETS ham_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/ham/base)

if(HAM_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()