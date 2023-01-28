#ifndef include_helpers_hpp
#define include_helpers_hpp

#include <chrono>

namespace chrono = std::chrono;

class Timer {
private:
  chrono::time_point<chrono::high_resolution_clock> start;
  chrono::time_point<chrono::high_resolution_clock> end;

public:
  void start_timer() { this->start = chrono::high_resolution_clock::now(); }
  void stop_timer() { this->end = chrono::high_resolution_clock::now(); }

  auto time_elapsed() {
    return chrono::duration_cast<chrono::microseconds>(this->end - this->start)
        .count();
  }
};

#endif