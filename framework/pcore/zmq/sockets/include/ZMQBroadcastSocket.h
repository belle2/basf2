#pragma once

#include <zmq.h>

namespace Belle2 {
  using ZMQBroadcastPubSocket = zmq::socket_t;
  using ZMQBroadcastSubSocket = zmq::socket_t;
}