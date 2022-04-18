cmake_minimum_required(VERSION 3.15)
project(Audinary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")
set(OBJS ${CMAKE_CURRENT_SOURCE_DIR}/deps/i18n.o ${CMAKE_CURRENT_SOURCE_DIR}/deps/beep.o)

include_directories(./include)

add_executable( audinary driver.cpp source/encoder/huffman-coding.cpp source/encoder/json-builder.cpp source/model/textual-time.cpp source/model/numbers-to-words.cpp source/server/http-server.cpp source/server/router.cpp)

target_link_libraries( audinary -lcAudio -pthread -lhttpserver -lfontconfig ${OBJS})
message(${CMAKE_CURRENT_SOURCE_DIR}/deps/internal_loc.o)
