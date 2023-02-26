#include <say_hello/application.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

void Application::init(const config& cfg) {
  _cfg = cfg;

  Server::config scfg{_cfg.port, _cfg.ip};
  _server.init(scfg);
}

void Application::start() {
  _server.start(*this);

  //Connect the client with next hop (same ip, port number +1)
  bool connected = false;
  while(!connected) {
    connected = _client.connect(_cfg.ip, _cfg.next_hop_port);
    std::this_thread::sleep_for(100us);
  }

  //App with id = 0 is the one to start the game
  if(_cfg.id == 0) {
    //Get randum number
    ++_counters.game_master;
    int n = rand() % _cfg.apps_count;
    _client.send(std::to_string(n));
  }

  while(!_done) {
    //logger
    std::cout << _counters;
    std::this_thread::sleep_for(2s);
  }
}

void Application::stop() {
  if(_done) return;
  _done = true;
  _server.shutdown();
  _client.shutdown();
}

//For now, apps exchange int number that represents target instance id
void Application::operator()(std::string&& data) {
  ++_counters.received_msgs;
  try{
    //Process the data
    printf("Application::operator() - received msg: %s\n", data.c_str());
    int n = std::atoi(data.c_str());
    if(n == _cfg.id) {
      printf("Application::operator() - My turn ;)\n");
      ++_counters.game_master;
      while(n == _cfg.id) n = rand() % _cfg.apps_count;
      std::this_thread::sleep_for(1s);
    } else {
      //nothing to do, just pass it to next hop
    }
    _client.send(std::to_string(n));
  } catch (const std::exception& ex) {
    printf("ERROR: Application::operator() - %s\n", ex.what());
  }
}