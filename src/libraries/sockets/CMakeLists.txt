add_library(client  client.cpp)
add_library(server  server.cpp)

target_include_directories(client PUBLIC ../../include/sockets)
target_include_directories(server PUBLIC ../../include/sockets)