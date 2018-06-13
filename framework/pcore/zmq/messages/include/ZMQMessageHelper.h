#pragma once

#include <framework/logging/LogMethod.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>
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


    static zmq::message_t createZMQMessage(const std::vector<char>& msgVector)
    {
      return zmq::message_t(msgVector.data(), msgVector.size());
    }


    static zmq::message_t createZMQMessage(const UniqueEventId& evtId)
    {
      std::string message = evtId.getSerial();
      //B2DEBUG(100, message);
      return zmq::message_t(message.c_str(), message.length());
    }


    static zmq::message_t createZMQMessage(const std::unique_ptr<EvtMessage>& evtMessage)
    {
      // here are the messages copied
      return zmq::message_t(evtMessage->buffer(), evtMessage->size());
    }

  };
}