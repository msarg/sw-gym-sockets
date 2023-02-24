#include <sockets/server.hpp>
#include <sockets/client.hpp>
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

class Application{
  public:
  Application()=default;
  ~Application()=default;
  void operator()(std::string&& msg) {
    printf("Got - %s\n", msg.data());
  }
};

int main(int argc, char** argv)
{
  signal(SIGPIPE, sig_handler);

  try {
    Server server;
    Application dummyApp;

    Server::config server_cfg;
    server_cfg.port = SERVER_PORT;
    server_cfg.ip = "127.0.0.1";
    server.init(server_cfg);
    server.start(dummyApp);
    // std::this_thread::sleep_for(1s);

    {
      Client client;
      client.connect("127.0.0.1", SERVER_PORT);
      if(!client.send("hello! I'm the client")) {
        throw std::runtime_error("Client cannot send.");
      }
      printf("client received: %s\n", client.read().data());
      std::this_thread::sleep_for(3s);

      Client client2;
      client2.connect("127.0.0.1", SERVER_PORT);
      if(!client2.send("hello! I'm the other client")) {
        throw std::runtime_error("Client cannot send.");
      }
      printf("client received: %s\n", client2.read().data());
    }
    std::this_thread::sleep_for(5s);

  } catch(std::exception& ex) {
    printf("Test failed: %s\n", ex.what());
  }
  return 0;
}