set(base_SRCS
  CountDownLatch.cc
  # CurrentThread.cc
  Exception.cc
  ThreadPool.cc
  Timestamp.cc
  )

add_library(ham_base ${base_SRCS})
target_link_libraries(ham_base pthread)

install(TARGETS ham_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/ham/base)

if(HAM_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()