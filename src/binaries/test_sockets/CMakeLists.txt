#project(test)

add_executable(test_client_server main.cpp)
target_link_libraries(test_client_server client server pthread)