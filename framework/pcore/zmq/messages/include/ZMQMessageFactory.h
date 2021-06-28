/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <memory>
#include <string>
#include <zmq.hpp>

namespace Belle2 {
/// Helper class for creating new ID/No-ID messages.
  class ZMQMessageFactory {
  public:
    /// Create an ID Message out of an identity, the type and an event message
    static auto createMessage(const std::string& msgIdentity,
                              const EMessageTypes msgType,
                              const std::unique_ptr<EvtMessage>& eventMessage)
    {
      return createMessage<ZMQIdMessage>(msgIdentity, msgType, eventMessage);
    }

    /// Create an ID Message out of an identity, the type and a string
    static auto createMessage(const std::string& msgIdentity,
                              const EMessageTypes msgType,
                              const std::string& msgData = "")
    {
      return createMessage<ZMQIdMessage>(msgIdentity, msgType, msgData);
    }

    /// Create an ID Message out of an identity, the type and another zmq message
    static auto createMessage(const std::string& msgIdentity,
                              const EMessageTypes msgType,
                              zmq::message_t msgData)
    {
      return createMessage<ZMQIdMessage>(msgIdentity, msgType, std::move(msgData));
    }

    /// Create an ID Message out of an identity and an already received message
    static auto createMessage(const std::string& msgIdentity,
                              std::unique_ptr<ZMQNoIdMessage> shortMessage)
    {
      return createMessage<ZMQIdMessage>(msgIdentity, std::move(shortMessage->getMessagePart<0>()),
                                         std::move(shortMessage->getMessagePart<1>()));
    }

    /// Create a No-ID Message out of an identity, the type and a string
    static auto createMessage(const EMessageTypes msgType,
                              const std::string& msgData = "")
    {
      return createMessage<ZMQNoIdMessage>(msgType, msgData);
    }

    /// Create a No-ID Message out of an identity, the type and an int
    static auto createMessage(const EMessageTypes msgType,
                              int msgData)
    {
      return createMessage<ZMQNoIdMessage>(msgType, msgData);
    }

    /// Create a No-ID Message out of an identity, the type and an event meta data
    static auto createMessage(const EMessageTypes msgType,
                              const StoreObjPtr<EventMetaData>& evtMetaData)
    {
      return createMessage<ZMQNoIdMessage>(msgType, evtMetaData);
    }

    /// Create a No-ID Message out of the type and another zmq message
    static auto createMessage(const EMessageTypes msgType,
                              zmq::message_t msgData)
    {
      return createMessage<ZMQNoIdMessage>(msgType, std::move(msgData));
    }

    /// Create a No-ID Message out of the type, another zmq message and an additional message
    static auto createMessage(const EMessageTypes msgType,
                              zmq::message_t msgData,
                              zmq::message_t additionalData)
    {
      return createMessage<ZMQNoIdMessage>(msgType, std::move(msgData), std::move(additionalData));
    }

    /// Create a No-ID Message out of an identity, the type and another zmq message
    static auto createMessage(zmq::message_t msgType,
                              zmq::message_t msgData)
    {
      return createMessage<ZMQNoIdMessage>(std::move(msgType), std::move(msgData));
    }

    /// Create a No-ID Message out of an identity, the type and an event message
    static auto createMessage(const EMessageTypes msgType,
                              const std::unique_ptr<EvtMessage>& eventMessage)
    {
      return createMessage<ZMQNoIdMessage>(msgType, eventMessage);
    }

    /// Create a No-ID Message out of an identity, the type, an event message, and and additional message
    static auto createMessage(const EMessageTypes msgType,
                              const std::unique_ptr<EvtMessage>& eventMessage,
                              zmq::message_t additionalData)
    {
      return createMessage<ZMQNoIdMessage>(msgType, eventMessage, std::move(additionalData));
    }

    /// Create a No-ID Message out of an ID message
    static auto stripIdentity(std::unique_ptr<ZMQIdMessage> message)
    {
      return createMessage<ZMQNoIdMessage>(std::move(message->getMessagePart<1>()),
                                           std::move(message->getMessagePart<2>()),
                                           std::move(message->getMessagePart<3>()));
    }

    /// Create a message of the given type by receiving a message from the socket.
    template <class AMessage>
    static std::unique_ptr<AMessage> fromSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      auto newMessage = std::unique_ptr<AMessage>(new AMessage());
      auto& messageParts = newMessage->getMessageParts();
      for (unsigned int i = 0; i < AMessage::c_messageParts; i++) {
        B2ASSERT("The next part does not belong to the same message",
                 socket->get(zmq::sockopt::rcvmore) == 1 or i == 0);
        auto received = socket->recv(messageParts[i], zmq::recv_flags::none);
        B2ASSERT("No message received", received);
      }
      B2ASSERT("There should not be more than the retrieved parts", socket->get(zmq::sockopt::rcvmore) == 0);
      return newMessage;
    }

  private:
    /// Small helper constructor to create a unique_ptr out of some arguments, as std::make_unique is not working with protected constructors
    template <class T, class... Args>
    static std::unique_ptr<T> createMessage(Args&& ... args)
    {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
  };
}
