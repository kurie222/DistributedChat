cmake_minimum_required(VERSION 3.0)
project(main)

# 编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置可执行文件存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 头文件
include_directories(${PROJECT_SOURCE_DIR}/include)
# 库文件
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 源文件列表
set(SRC_LIST test_muduo.cpp)

# 把指定路径下的放入
aux_source_directory(. SRC_LIST)

add_executable(server ${SRC_LIST})

target_link_libraries(server muduo_net muduo_base pthread)