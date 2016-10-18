#pragma once

#include <string>

namespace Belle2 {
  std::string random_name(size_t length, bool only_digits = false);

  std::string random_socket_name(bool port, const std::string& hostname = "127.0.0.1");
};
