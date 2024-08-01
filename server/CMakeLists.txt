#定义服务器端的子目录构建文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(EpollServer ${SRC_LIST}
        EchoServer.cpp
        src/Timer.cpp
        include/Timer.h
        include/TimerId.h
        src/TimerQueue.cpp
        include/TimerQueue.h
)

# 添加需要的库（例如网络库）
# target_link_libraries(EpollServer ${SOME_LIBRARY})