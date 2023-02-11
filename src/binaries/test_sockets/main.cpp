#include "../../include/sockets/sockets/server.hpp"
#include "../../include/sockets/sockets/client.hpp"
#include <chrono>
#include <exception>
#include <signal.h>
#include <sstream>

using namespace std::chrono_literals;

void sig_handler(int sig) {
  std::ostringstream msg;
  msg << "sig_handler() - captured signal " << sig << "\n";
  std::cout << msg.str();
}
uint16_t SERVER_PORT{2004};

int main(int argc, char** argv)
{
  signal(SIGPIPE, sig_handler);
  try {
    
    Client client;
    Server server;

    Server::config server_cfg;
    server_cfg.port = SERVER_PORT;
    server.init(server_cfg);
    server.start();
    client.connect("127.0.0.1", SERVER_PORT);
    client.send("hello! I'm the client");

  } catch(std::exception& ex) {
    std::cout << "Exception: " << ex.what() << std::endl;
  }
  return 0;
}