#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <unordered_map>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>

#include "logger.hpp"

// class IP;
using IP = std::string; //TODO: to be a class
using port_t = uint16_t; //port is a 16-bit number (0 : 65535) ("Ports 0-1023 are reserved by the system and used by common network protocols.")

const uint32_t BACKLOG_LENGTH = 5;


class Server{
public:
  struct config{
    port_t port;
    IP ip;
  };

  struct counters {
    std::atomic<uint64_t> received_bytes{0};
    std::atomic<uint64_t> received_msgs{0};
    std::atomic<uint64_t> clients_count{0};
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

  template<typename T>
  void start(T& app);

  const counters& get_counters() const noexcept {
    return _counters;
  }

private:
  int _get_master_socket();
  bool _set_address(sockaddr_in& addr);
  std::pair<fd_set, int> _set_fds();

  config _cfg;
  counters _counters;
  std::atomic<bool> _done{false};
  std::thread _server;
  int _master_sock{-1};
  std::unordered_map<int, struct sockaddr_in> _fds;
  std::mutex _fds_door;
};


template<typename T>
void Server::start(T& app) {
  using namespace std::chrono_literals;
  std::thread([this](){
    if(_master_sock == -1) {
      std::cout << "Fatal: server doesn't have a valid master socket fd! exit.\n";
      return;
    }

    try {
      while(!_done) {
        //OK, accept the connection
        struct sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        /*The addrlen argument is a value-result argument. It points to an integer that,
          prior to the call, must be initialized to the size of the buffer pointed to by addr, so
          that the kernel knows how much space is available to return the socket address.
          Upon return from accept(), this integer is set to indicate the number of bytes of data
          actually copied into the buffer.*/
        uint32_t src_addr_len = sizeof(client_addr);
        /* accept() is a blocking function - it returns an "active socket*/
        const int new_conn_fd = accept(_master_sock, (struct sockaddr*)&client_addr, (socklen_t*)&src_addr_len);
        if( new_conn_fd == -1 ) {
          perror("Server - accept");
          continue;
        }

        ++_counters.clients_count;
        //Print requester info
        logger::info("Server::start() - accepted new connection: IP %s, , port: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        {
          //Set it in non-blocking mode
          auto flags = fcntl(new_conn_fd, F_GETFL);
          fcntl(new_conn_fd, flags, O_NONBLOCK);
          std::lock_guard<std::mutex> lock{_fds_door};
          _fds.insert({new_conn_fd, client_addr});
        }
      }

      std::cout << "Server::start() - end." << std::endl;

    } catch (std::exception& ex) {
      logger::error("server ex: %s\n", ex.what());
      return;
    }
  }).detach();

  //Second thread: check fds
  std::thread([&app, this](){
    
    struct timeval timeout{0, 0};
    while(!_done) {
      try {
        auto [fds_set, nfd] = _set_fds();

        int rslt = select(nfd, &fds_set, NULL, NULL, &timeout);
        if(rslt < 0) {
          perror("select");
        }
        if(rslt > 0) {
          for(int i =0; i < nfd; ++i) {
            if(FD_ISSET(i, &fds_set)) {
              char buffer[1024];
              int count = read(i, buffer, 256); //reads upto 256 bytes

              if (count > 0) {
                _counters.received_bytes += count;
                ++_counters.received_msgs;

                //Move the msg to the upstream app
                app(std::move(std::string(buffer, count)));
              } else {
                //Ready with no data to read means client closed the connection
                logger::debug("Closing fd %d\n", i);
                close(i);
                std::lock_guard<std::mutex> lock{_fds_door};
                _fds.erase(i);
              }
            }
          }
        } else {
          //no ready fds
          std::this_thread::sleep_for(100us); //TODO: move it to timeout
        }
      } catch(const std::exception& ex) {
        logger::error("select thread - exception %s", ex.what());
      }
    }
  }).detach();

  //Wait for a while until server is up
  std::this_thread::sleep_for(100us);
}
