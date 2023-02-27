#pragma once

#include <iostream>
#include <string>
#include <atomic>

namespace logger {
  enum levels{
    INFO,
    DEBUG
  };

  static std::atomic<uint8_t> _level{0};
  static void set_level(const uint8_t level) {
    _level = level;
  }

  template<typename... Args>
  static void _log(const char* prefix, const Args&... args) {
    char buffer[1024];
    sprintf(buffer, args...);
    printf("%s: %s\n", prefix, buffer);
  }

  template<typename T>
  static void _log(const char* prefix, const T& t) {
    printf("%s: %s\n", prefix, t);
  }

  template<typename... Args>
  static void info(const Args&... args) {
    _log("INFO",  args...);
  }

  template<typename... Args>
  static void war(const Args&... args) {
    _log("WARN",  args...);
  }

  template<typename... Args>
  static void error(const Args&... args) {
    _log("ERROR",  args...);
  }

  template<typename... Args>
  static  void debug(const Args&... args) {
    if(_level == DEBUG) _log("DEBUG",  args...);
  }
}