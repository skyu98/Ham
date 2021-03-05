set(net_SRCS
  Channel.cc
  EventLoop.cc
  Epoller.cc
  SocketOps.cc
  Timer.cc
  TimerQueue.cc
  Acceptor.cc
  Socket.cc
  InetAddress.cc
  Buffer.cc
  TcpServer.cc
  TcpConnection.cc
  EventLoopThread.cc
  EventLoopThreadPool.cc
  )

add_library(ham_net ${net_SRCS})
target_link_libraries(ham_net ham_base)

install(TARGETS ham_net DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/ham/net)

if(HAM_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()

add_subdirectory(http)