file(GLOB PRTBACKEND_SRCS "*.c" "*.cpp")
# list(APPEND PRTBACKEND_SRCS ${DIR_SRCS})
# list(FILTER PRTBACKEND_SRCS EXCLUDE REGEX "main.cpp")
# list(FILTER PRTBACKEND_SRCS EXCLUDE REGEX "client.cpp")

add_executable(prtbackend ${PRTBACKEND_SRCS})

target_compile_options(prtbackend PRIVATE -O2)

target_include_directories(prtbackend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(prtbackend PRIVATE ../include)
target_include_directories(prtbackend PRIVATE ../3rdparty)