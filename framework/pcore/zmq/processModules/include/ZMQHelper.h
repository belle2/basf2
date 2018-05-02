#pragma once

#include <zmq.hpp>
#include <framework/core/Environment.h>

namespace Belle2 {
  class ZMQHelper {
  public:
    static bool pollSocket(std::unique_ptr<zmq::socket_t>& socket, int timeout)
    {
      zmq::pollitem_t items [] = {
        { static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0 }
      };
      zmq::poll(&items[0], 1, timeout);
      return static_cast<bool>(items [0].revents & ZMQ_POLLIN);
    }

  };
}
