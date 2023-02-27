#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <sockets/logger.hpp>
#include <sockets/client.hpp>

Client::~Client() {
  std::cout << "client shutdown\n";
  if (_sock != -1) {
    close(_sock);
  }
  std::cout << "client shutdown - Done\n";
};

bool Client::connect(const std::string& server_ip, const uint16_t server_port) {
  std::cout << "Client::connect()\n";

  //you need a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1) {
    perror("socket creation failed!");
    return false;
  }

  logger::info("Client::connect() - socket fd: %d\n", sock);

  //Connect on this socket

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  // server_addr.sin_addr.s_addr = INADDR_ANY;
  if(inet_pton(AF_INET, server_ip.data() , &server_addr.sin_addr) != 1){
    std::cout << "Error: cannot convert IP to network address.\n";
    return false;
  }

  if(::connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    //TODO: do we need to retry? how many?
    perror("connect");
    return false;
  }

  _sock = sock;
  return true;
}

bool Client::send(const std::string msg) {
  if(_sock == -1) {
    std::cout << "Cannot send; no valid socket!\n";
    return false;
  }
  const int count = ::send(_sock, msg.data(), msg.size() +1, 0);
  if(count > 0) {
    logger::info("Client::send() - sent %d bytes\n", count);
    return true;
  } else {
    std::cout << "Client cannot send!\n";
    return false;
  }
}

std::string Client::read() {
  char buffer[1024];
  int count = ::read(_sock, buffer, 1024);
  return std::string(buffer, count);
}

void Client::shutdown() {
  if(_sock == -1) return;
  close(_sock);
  _sock = -1;
}