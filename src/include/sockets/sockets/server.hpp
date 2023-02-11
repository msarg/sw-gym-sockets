#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
// #include <format>

using namespace std::chrono_literals;

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

  void init(const config& cfg) {
    _port = cfg.port;
    _ip = cfg.ip;

    /*
    * creates master socket fd.
    */
    _master_sock = _get_master_socket();
    if(_master_sock == -1) {
      perror("failed to create a master socket!");
      return;
    }
    /*
    * "listen() marks the socket referred to by sockfd as a passive socket,
    * that is, as a socket that will be used to accept incoming connection requests using accept()."
    */
    //"calls listen() to notify the kernel of its willingness to accept incoming connections."
    if( listen(_master_sock, BACKLOG_LENGTH) < -1 ) {
      std::cout << "listen failed" << std::endl;
      exit(1);
    } else {
      printf("Server::start() - listening fd : %d\n", _master_sock);
    }
  }

  void shutdown() {
    std::cout << "Server::shutdown()\n";
    _done = true;

    if(_master_sock > 0) {
      close(_master_sock);
      _master_sock = -1;
    }
    std::cout << "Server::shutdown() - done\n";
  }

  //TODO: may need to take pointer to a callable app class to pass data to

  void start() {
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

          //Print requester info
          printf("Server::start() - accepted new connection: IP %s, , port: %d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

          //Now, what to do with this new connection fd? 
          std::string msg{"Hello from the server!"};
          int count = send(new_conn_fd, msg.data(), msg.size()+1, 0 /*flags*/);
          if(count > 0) {
            printf("Server::start() - succssfully sent: %d bytes\n", count);
          }

          char buffer[1024];
          count = read(new_conn_fd, buffer, 256); //reads upto 256 bytes
          if (count > 0) {
            printf("Server::start() - msg read: %s\n", std::string(buffer, count).data());
          }

          close(new_conn_fd);
        }

        std::cout << "Server::start() - end." << std::endl;

      } catch (std::exception& ex) {
        printf("server ex: %s\n", ex.what());
        return;
      }
    }).detach();
  }

private:
  int _get_master_socket()
  {
    const int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(sock_fd < 0) {
      perror("Server - master socket creation failed!");
      return -1;
    } 

    /* Now, let us use the master server fd to bind to the port
     * "bind() assigns the address specified by addr to the socket referred to
       by the file descriptor sockfd.
       addrlen specifies the size, in bytes, of the address structure pointed to by addr.
       Traditionally, this operation is called “assigning a name to a socket”."
    */
    sockaddr_in addr;
    if(!_set_address(addr)) {
      std::cout << "Fatal: Server - cannot set addess\n";
      return -1;
    }

    if(bind(sock_fd, (struct sockaddr*)(&addr), sizeof(addr)) < 0) {
      perror("bind");
      return -1; //FIXME: what about the already created socket? are not we responsible for closing it?
    }

    return sock_fd;
  }

  bool _set_address(sockaddr_in& addr)
  {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    return inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == 1;
  }

  port_t _port{0};
  IP _ip;
  std::atomic<bool> _done{false};
  std::thread _server;
  int _master_sock{-1};
};