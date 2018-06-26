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

    static zmq::message_t createZMQMessage(const int i)
    {
      return createZMQMessage(std::to_string(i));
    }

    static zmq::message_t createZMQMessage(const StoreObjPtr<EventMetaData>& evtMetaData)
    {
      std::string message = std::to_string(evtMetaData->getEvent()) + ":" +
                            std::to_string(evtMetaData->getRun()) + ":" +
                            std::to_string(evtMetaData->getExperiment());
      return zmq::message_t(message.c_str(), message.length());
    }


    static zmq::message_t createZMQMessage(const std::unique_ptr<EvtMessage>& evtMessage)
    {
      // TODO: here are the messages copied
      return zmq::message_t(evtMessage->buffer(), evtMessage->size());
    }

  };
}