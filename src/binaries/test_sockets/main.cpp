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
    uint64_t sent_bytes{0};

    {
      Client client;
      client.connect("127.0.0.1", SERVER_PORT);
      const std::string msg1{"hello! I'm the client"};
      if(!client.send(msg1)) {
        throw std::runtime_error("Client cannot send.");
      }
      sent_bytes += msg1.size() +1;
      std::this_thread::sleep_for(10ms);
      if( server.get_counters().received_bytes != sent_bytes ) {
        printf("server::received_bytes: %ld, sent_bytes: %ld\n", server.get_counters().received_bytes.load(), sent_bytes );
        throw;
      }
      std::this_thread::sleep_for(3s);

      Client client2;
      client2.connect("127.0.0.1", SERVER_PORT);
      const std::string msg2{"hello! I'm the other client"};
      if(!client2.send(msg2)) {
        throw std::runtime_error("Client cannot send.");
      }
      sent_bytes += msg2.size() +1;
      std::this_thread::sleep_for(10ms);
      if( server.get_counters().received_bytes != sent_bytes ) {
        printf("server::received_bytes: %ld, sent_bytes: %ld\n", server.get_counters().received_bytes.load(), sent_bytes);
        throw;
      }
    }
    std::this_thread::sleep_for(5s);

  } catch(std::exception& ex) {
    printf("TEST FAILED: %s\n", ex.what());
  }
  printf("TEST PASSED!");
  return 0;
}