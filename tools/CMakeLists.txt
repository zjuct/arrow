file(GLOB OBBGEN_SRCS "*.c" "*.cpp")
list(APPEND OBBGEN_SRCS ${DIR_SRCS})
list(FILTER OBBGEN_SRCS EXCLUDE REGEX "main.cpp")

# message(AUTHOR_WARNING "source files: ${OBBGEN_SRCS}")

add_executable(obbgen ${OBBGEN_SRCS})

target_include_directories(obbgen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(obbgen PRIVATE ../include)
target_include_directories(obbgen PRIVATE ../3rdparty)

target_link_libraries(obbgen PRIVATE ${A})