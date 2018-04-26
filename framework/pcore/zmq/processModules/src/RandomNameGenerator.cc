#include <framework/pcore/zmq/processModules/RandomNameGenerator.h>
#include <TRandom.h>
#include <algorithm>
#include <fstream>

namespace Belle2 {
  std::string random_name(size_t length, bool only_digits)
  {
    auto randchar = [&only_digits]() -> char {
      if (only_digits)
      {
        const char charset[] = "0123456789";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[gRandom->Integer(max_index)];
      } else {
        const char charset[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[gRandom->Integer(max_index)];
      }
    };

    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);

    return str;
  }

  std::string random_socket_name(bool port, const std::string& hostname)
  {
    if (port) {
      std::string socket_name = hostname + ":" + random_name(5, port);
      return socket_name;
    } else {
      std::string socket_name = "." + random_name(10, false) + ".socket";
      while (std::ifstream(socket_name)) {
        socket_name = "." + random_name(10, false) + ".socket";
      }

      return socket_name;
    }
  }
}
