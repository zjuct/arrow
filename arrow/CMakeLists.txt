cmake_minimum_required(VERSION 3.10)
project(arrow)

# 设置编译选项
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 0)

# 设置源文件目录
aux_source_directory(./src DIR_SRCS)
# set(DIR_SRCS ./src/main.cpp)

message(AUTHOR_WARNING "source files: ${DIR_SRCS}")

include_directories(./include)
include_directories(./3rdparty)
message(AUTHOR_WARNING "include path: ${CMAKE_CURRENT_SOURCE_DIR}/include")


# link_directories(./lib)
message(AUTHOR_WARNING "link path: ${CMAKE_CURRENT_SOURCE_DIR}/lib")

# 添加源文件
add_executable(arrow ${DIR_SRCS})

find_library(A glfw3 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
message(AUTHOR_WARNING "glfw3 path: ${A}")

target_link_libraries(arrow 
    PUBLIC ${A}
)


