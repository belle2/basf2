#pragma once

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/EvtMessage.h>
#include <zmq.hpp>
#include <string>
#include <memory>

namespace Belle2 {

  class ZMQMessageHelper {
  public:
    static zmq::message_t createZMQMessage(const c_MessageTypes& c)
    {
      return createZMQMessage(std::string(1, static_cast<char>(c)));
    }

    static zmq::message_t createZMQMessage(const std::string& s)
    {
      return zmq::message_t(s.c_str(), s.length());
    }

    static zmq::message_t createZMQMessage(const std::unique_ptr<EvtMessage>& evtMessage)
    {
      return zmq::message_t(evtMessage->buffer(), evtMessage->size());
    }

  };
}