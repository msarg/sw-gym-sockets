#include <string>
#include <sstream>


#include <sockets/server.hpp>

using namespace std::chrono_literals;

void Server::init(const config& cfg) {
  _cfg = cfg;

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

void Server::shutdown() {
  std::cout << "Server::shutdown()\n";
  _done = true;

  if(_master_sock > 0) {
    close(_master_sock);
    _master_sock = -1;
  }
  std::cout << "Server::shutdown() - done\n";
}


int Server::_get_master_socket()
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

bool Server::_set_address(sockaddr_in& addr)
{
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_cfg.port);
  return inet_pton(AF_INET, _cfg.ip.c_str(), &addr.sin_addr) == 1;
}

std::pair<fd_set, int> Server::_set_fds() {
  //Loop over fds and add them
  fd_set fds_set;
  FD_ZERO(&fds_set);
  int nfd = 0;
  std::lock_guard<std::mutex> lock{_fds_door};
  for(const auto& elem: _fds){
    int fd = elem.first;
    FD_SET(fd, &fds_set);
    if(fd >= nfd) nfd = fd +1;
  }
  return {fds_set, nfd};
}