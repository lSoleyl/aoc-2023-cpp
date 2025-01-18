#include <chrono>
#include <iostream>
#include <optional>

namespace common {
  struct Time {
    using clock = std::chrono::high_resolution_clock;

    Time() : t1(clock::now()) {}


    void completed() {
      if (!done) {
        t2 = clock::now();
      }
    }

    clock::time_point t1, t2;
    bool done = false;
  };

  std::ostream& operator<<(std::ostream& out, Time& t) {
    t.completed(); // complete unless already marked as completed
    return out << "Time " << std::chrono::duration_cast<std::chrono::milliseconds>(t.t2 - t.t1).count() << "ms\n";
  }
}