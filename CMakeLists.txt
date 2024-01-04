cmake_minimum_required(VERSION 3.10)
project(arrow)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX)
	set(UNIX_LIBS -lGL -lX11 -lpthread -lXrandr -LXi -ldl)
else()
	set(UNIX_LIBS "")
endif()

if(WIN32)
	set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)
endif()

# 设置编译选项
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 0)


# add_compile_options(-DSAT_TEST)

# 设置源文件目录
aux_source_directory(./src DIR_SRCS)
aux_source_directory(./test TEST_SRCS)
aux_source_directory(./server SERVER_SRCS)

list(TRANSFORM DIR_SRCS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/" OUTPUT_VARIABLE DIR_SRCS)

# 添加源文件
add_executable(arrow ${DIR_SRCS} ${TEST_SRCS})

target_include_directories(arrow PRIVATE ./include)
target_include_directories(arrow PRIVATE ./3rdparty)
target_include_directories(arrow PRIVATE ./test)

find_library(A glfw3 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
find_library(B freetype PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
message(AUTHOR_WARNING "${B}")

target_link_libraries(arrow 
    PUBLIC ${A}
		PUBLIC ${B}
		${UNIX_LIBS}
		ws2_32
)


# 设置编译选项 -Wno-volatile
target_compile_options(arrow PRIVATE -Wno-volatile)


set_target_properties(arrow PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  	# add_subdirectory(tools)
	add_subdirectory(server)
	# add_executable(server ${DIR_SRCS} ${TEST_SRCS} ${SERVER_SRCS})
	# target_include_directories(server PRIVATE ./include)
	# target_include_directories(server PRIVATE ./3rdparty)
	# target_include_directories(server PRIVATE ./test)

	set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_subdirectory(prtbackend)
set_target_properties(prtbackend PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

