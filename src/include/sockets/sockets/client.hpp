#include <string>

class Client {
public:
  Client() = default;
  ~Client();

  void shutdown();
  bool connect(const std::string& server_ip, const uint16_t server_port);
  bool send(const std::string msg);
  std::string read();

private:
  // uint16_t _port{0};
  int _sock = -1;
};