set(http_SRCS
  HttpContext.cc
  HttpRequest.cc
  HttpResponse.cc
  HttpServer.cc
  )

add_library(ham_http ${net_SRCS})
target_link_libraries(ham_http ham_net ham_base)

install(TARGETS ham_http DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/ham/http)

if(HAM_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()
