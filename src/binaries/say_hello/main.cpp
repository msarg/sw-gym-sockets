#include <iostream>
#include <signal.h>

#include <sockets/client.hpp>
#include <sockets/server.hpp>
#include <say_hello/application.hpp>

void sig_handler(int s) {
  printf("Catched signal %d\n", s);
  exit(1);
}

int main(int argc, char** argv)
{
  signal(SIGINT, sig_handler);

  try {
    Application app;
    Application::config cfg;

    //parse arguments
    if(argc != 6) {
      throw std::runtime_error("invalid arguments! args: id IP port next_port apps_count");
    }
    cfg.id = std::atoi(argv[1]);
    cfg.ip = argv[2];
    cfg.port = std::atoi(argv[3]);
    cfg.next_hop_port = std::atoi(argv[4]);
    cfg.apps_count = std::atoi(argv[5]);

    app.init(cfg);
    app.start();
  } catch (const std::exception& ex) {
    printf("ERROR: %s\n", ex.what());
    return 1;
  }

  return 0;
}