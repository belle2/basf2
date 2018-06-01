#pragma once

#include <zmq.hpp>
#include <framework/core/Environment.h>
#include <iostream>
#include <time.h>



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


    static int pollSockets(std::vector<zmq::socket_t*>& socketList, int timeout)
    {
      int return_bitmask = 0;
      zmq::pollitem_t items [socketList.size()];

      for (unsigned int i = 0; i < socketList.size(); i++) {
        items[i].socket = static_cast<void*>(*socketList[i]);
        items[i].events = ZMQ_POLLIN;
        items[i].revents = 0;
      }

      zmq::poll(items, socketList.size(), timeout);

      for (unsigned int i = 0; i < socketList.size(); i++) {
        if (static_cast<bool>(items [i].revents & ZMQ_POLLIN))
          return_bitmask = return_bitmask | 1 << i;
      }
      return return_bitmask;
    }


    static std::string getSocketAddr(std::string socketName, std::string socketProtocol)
    {
      return socketProtocol + "://" + socketName;
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
            if (!more) { break; }
          }

        }
        if (difftime(time(NULL), startTime) > timeoutValue) {
          return;
        }
      }
    }
  };
}
