add_library(say_hello application.cpp)
target_link_libraries(say_hello server client)

target_include_directories(say_hello PUBLIC ../../include/say_hello)