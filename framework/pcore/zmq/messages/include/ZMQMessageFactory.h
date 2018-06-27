#pragma once

#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/logging/LogMethod.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <memory>
#include <string>
#include <zmq.hpp>

namespace Belle2 {
  /// Helper class for creating new ID/No-ID messages.
  class ZMQMessageFactory {
  public:
    /// Create an ID Message out of an identity, the type and an event message
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::unique_ptr<EvtMessage>& eventMessage)
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, msgType, eventMessage));
    }

    /// Create an ID Message out of an identity, the type and a string
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, msgType, msgData));
    }


    /// Create a No-ID Message out of an identity, the type and a string
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, msgData));
    }

    /// Create a No-ID Message out of an identity, the type and an int
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         int msgData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, msgData));
    }

    /// Create a No-ID Message out of an identity, the type and an event meta data
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const StoreObjPtr<EventMetaData>& evtMetaData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, evtMetaData));
    }

    /// Create a No-ID Message out of an identity, the type and an event message
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType, const std::unique_ptr<EvtMessage>& eventMessage)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, eventMessage));
    }

    /// Create a message of the given type by receiving a message from the socket.
    template <class AMessage>
    static std::unique_ptr<AMessage> fromSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      auto newMessage = std::unique_ptr<AMessage>(new AMessage());
      auto& messageParts = newMessage->getMessageParts();
      for (unsigned int i = 0; i < AMessage::c_messageParts; i++) {
        socket->recv(&messageParts[i]);
      }
      return newMessage;
    }
  };
}