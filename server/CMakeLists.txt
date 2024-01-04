file(GLOB SERVER_SRCS "*.c" "*.cpp")
list(APPEND SERVER_SRCS ${DIR_SRCS})
list(FILTER SERVER_SRCS EXCLUDE REGEX "main.cpp")
list(FILTER SERVER_SRCS EXCLUDE REGEX "client.cpp")

# message(AUTHOR_WARNING "source files: ${OBBGEN_SRCS}")

add_executable(server ${SERVER_SRCS})

target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(server PRIVATE ../include)
target_include_directories(server PRIVATE ../3rdparty)

message(AUTHOR_WARNING "A: ${A}")
target_link_libraries(server 
    PUBLIC ${A} 
    PUBLIC ${B}
    ws2_32
)

target_compile_definitions(server PRIVATE SERVER)