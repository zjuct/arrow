cmake_minimum_required(VERSION 3.10)
project(arrow)

# 设置目标为Mingw
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)



# 设置编译选项
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 0)

set(CMAKE_BUILD_TYPE "Debug")

# add_compile_options(-DSAT_TEST)

# 设置源文件目录
aux_source_directory(./src DIR_SRCS)
aux_source_directory(./test TEST_SRCS)
aux_source_directory(./server SERVER_SRCS)
aux_source_directory(./src/utils UTIL_SRCS)

list(TRANSFORM DIR_SRCS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/" OUTPUT_VARIABLE DIR_SRCS)
# set(DIR_SRCS ./src/main.cpp)

# message(AUTHOR_WARNING "source files: ${DIR_SRCS}")

# message(AUTHOR_WARNING "include path: ${CMAKE_CURRENT_SOURCE_DIR}/include")


# link_directories(./lib)
# message(AUTHOR_WARNING "link path: ${CMAKE_CURRENT_SOURCE_DIR}/lib")

# 添加源文件
add_executable(arrow ${DIR_SRCS} ${TEST_SRCS} ${UTIL_SRCS})

target_include_directories(arrow PRIVATE ./include)
target_include_directories(arrow PRIVATE ./3rdparty)
target_include_directories(arrow PRIVATE ./test)

find_library(A glfw3 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# message(AUTHOR_WARNING "glfw3 path: ${A}")

target_link_libraries(arrow 
    PUBLIC ${A}
)

# 设置编译选项 -Wno-volatile
target_compile_options(arrow PRIVATE -Wno-volatile)

set_target_properties(arrow PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(tools)

add_executable(server ${SERVER_SRCS})

set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

