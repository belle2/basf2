#pragma once

#include <zmq.hpp>
#include <framework/core/Environment.h>
#include <iostream>
#include <time.h>

#include <framework/pcore/zmq/processModules/RandomNameGenerator.h>

namespace Belle2 {
  enum ZMQAddressType {
    c_input,
    c_output,
    c_pub,
    c_sub,
    c_control
  };

  class ZMQHelper {

  public:
    static bool pollSocket(const std::unique_ptr<zmq::socket_t>& socket, int timeout)
    {
      zmq::pollitem_t items[] = {
        {static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0}
      };
      try {
        zmq::poll(&items[0], 1, timeout);
        return static_cast<bool>(items[0].revents & ZMQ_POLLIN);
      } catch (zmq::error_t error) {
        if (error.num() == EINTR) {
          return false;
        } else {
          throw error;
        }
      }
    }

    static int pollSockets(const std::vector<zmq::socket_t*>& socketList, int timeout)
    {
      int return_bitmask = 0;
      zmq::pollitem_t items[socketList.size()];

      for (unsigned int i = 0; i < socketList.size(); i++) {
        items[i].socket = static_cast<void*>(*socketList[i]);
        items[i].events = ZMQ_POLLIN;
        items[i].revents = 0;
      }

      try {
        zmq::poll(items, socketList.size(), timeout);

        for (unsigned int i = 0; i < socketList.size(); i++) {
          if (static_cast<bool>(items[i].revents & ZMQ_POLLIN))
            return_bitmask = return_bitmask | 1 << i;
        }
        return return_bitmask;
      } catch (zmq::error_t error) {
        if (error.num() == EINTR) {
          return -1;
        } else {
          throw error;
        }
      }
    }

    static std::string getSocketAddress(const std::string& socketAddress, const ZMQAddressType socketPart)
    {
      if (socketPart == ZMQAddressType::c_input) {
        return socketAddress + "_input";
      } else if (socketPart == ZMQAddressType::c_output) {
        return socketAddress + "_output";
      } else if (socketPart == ZMQAddressType::c_pub) {
        return socketAddress + "_pub";
      } else if (socketPart == ZMQAddressType::c_sub) {
        return socketAddress + "_sub";
      } else if (socketPart == ZMQAddressType::c_control) {
        return socketAddress + "_control";
      }
      return socketAddress;
    }

    static void socketSniffer(std::unique_ptr<zmq::socket_t>& socket, int timeout, int timeoutValue = 20)
    {
      time_t startTime = time(NULL);
      while (1) {
        if (pollSocket(socket, timeout)) {
          while (1) {
            zmq::message_t message;
            socket->recv(&message);
            std::cout << "Message part " << std::string(static_cast<const char*>(message.data()), message.size())
                      << std::endl;
            int more;
            size_t more_size = sizeof(more);
            socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
            if (!more) {
              break;
            }
          }
        }
        if (difftime(time(NULL), startTime) > timeoutValue) {
          return;
        }
      }
    }
  };
} // namespace Belle2
