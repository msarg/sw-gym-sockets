#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <unordered_map>

#include <netinet/in.h>


// class IP;
using IP = uint32_t; //TODO: to be a class
using port_t = uint16_t; //port is a 16-bit number (0 : 65535) ("Ports 0-1023 are reserved by the system and used by common network protocols.")

const uint32_t BACKLOG_LENGTH = 5;


class Server{
public:
  struct config{
    port_t port;
    IP ip;
  };

  Server() = default;
  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(const Server&) = delete;
  Server& operator=(Server&&) = delete;
  ~Server() {
    shutdown();
  };

  void init(const config& cfg);
  void shutdown();
  //TODO: may need to take pointer to a callable app class to pass data to
  void start();

private:
  int _get_master_socket();
  bool _set_address(sockaddr_in& addr);

  port_t _port{0};
  IP _ip;
  std::atomic<bool> _done{false};
  std::thread _server;
  int _master_sock{-1};
  std::unordered_map<int, struct sockaddr_in> _fds;
  std::mutex _fds_door;
};