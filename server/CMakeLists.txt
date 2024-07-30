#定义服务器端的子目录构建文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(EpollServer ${SRC_LIST}
        src/Buffer.cpp
        include/Buffer.h
        src/TimeStamp.cpp
        include/TimeStamp.h
        include/logger.h
        src/logger.cpp
        src/Poller.cpp
        include/Poller.h
        src/DefaultPoller.cpp
        include/EPollPoller.h
        src/CurrentThread.cpp
        include/CurrentThread.h
        src/Thread.cpp
        include/Thread.h
        include/nocopyable.h
        src/EventLoopThread.cpp
        include/EventLoopThread.h
        src/EventLoopThreadPool.cpp
        include/EventLoopThreadPool.h
        include/callbacks.h
        EchoServer.cpp
)

# 添加需要的库（例如网络库）
# target_link_libraries(EpollServer ${SOME_LIBRARY})