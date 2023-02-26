#pragma once

#include <iostream>

#include <sockets/server.hpp>
#include <sockets/client.hpp>

class Application{
public:
  struct config {
    uint8_t id;
    uint8_t apps_count;
    std::string ip;
    uint16_t port;
    uint16_t next_hop_port;
  };

  struct counters {
    std::atomic<uint64_t> received_msgs{0};
    std::atomic<uint64_t> game_master{0};

    friend std::ostream& operator<<(std::ostream& os, const struct counters& rhs) {
      os << "received_msgs: " << rhs.received_msgs << "\n"
         << "game_master  : " << rhs.game_master   << "\n";
      return os;
    }
  };

  Application() = default;
  ~Application() {
    stop();
  }

  void init(const config& cfg);
  void start();
  void stop();

  void operator()(std::string&& data);

private:
  config _cfg;
  counters _counters;
  Server _server;
  Client _client;
  std::atomic<bool> _done{false};
};