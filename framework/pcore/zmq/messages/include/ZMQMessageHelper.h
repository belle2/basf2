#pragma once

#include <framework/logging/LogMethod.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <zmq.hpp>
#include <string>
#include <memory>
#include <vector>

namespace Belle2 {

  /// Internal helper for creating ZMQ messages (should not be used otherwise).
  class ZMQMessageHelper {
  public:
    /// Create a message out of a type by casting to a char
    static zmq::message_t createZMQMessage(const c_MessageTypes& c)
    {
      return createZMQMessage(std::string(1, static_cast<char>(c)));
    }

    /// Create a message out of a string
    static zmq::message_t createZMQMessage(const std::string& s)
    {
      return zmq::message_t(s.c_str(), s.length());
    }

    /// Create a message out of an int by casting to a string
    static zmq::message_t createZMQMessage(const int i)
    {
      return createZMQMessage(std::to_string(i));
    }

    /// Create a message out of an event meta data by serialization. TODO
    static zmq::message_t createZMQMessage(const StoreObjPtr<EventMetaData>& evtMetaData)
    {
      std::string message = std::to_string(evtMetaData->getEvent()) + ":" +
                            std::to_string(evtMetaData->getRun()) + ":" +
                            std::to_string(evtMetaData->getExperiment());
      return zmq::message_t(message.c_str(), message.length());
    }

    /// Create a message out of an event message.
    static zmq::message_t createZMQMessage(const std::unique_ptr<EvtMessage>& evtMessage)
    {
      // TODO: here are the messages copied
      return zmq::message_t(evtMessage->buffer(), evtMessage->size());
    }

  };
}