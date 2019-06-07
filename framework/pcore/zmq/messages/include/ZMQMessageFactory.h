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
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage({
        ZMQMessageHelper::createZMQMessage(msgIdentity),
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(eventMessage)
      }));
    }

    /// Create an ID Message out of an identity, the type and a string
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage({
        ZMQMessageHelper::createZMQMessage(msgIdentity),
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(msgData)
      }));
    }

    /// Create an ID Message out of an identity, the type and another zmq message
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       zmq::message_t msgData)
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage({
        ZMQMessageHelper::createZMQMessage(msgIdentity),
        ZMQMessageHelper::createZMQMessage(msgType),
        std::move(msgData)
      }));
    }

    /// Create an ID Message out of an identity and an already received message
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       std::unique_ptr<ZMQNoIdMessage> shortMessage)
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage({
        ZMQMessageHelper::createZMQMessage(msgIdentity),
        std::move(shortMessage->getMessagePart<0>()),
        std::move(shortMessage->getMessagePart<1>())
      }
                                                           ));
    }


    /// Create a No-ID Message out of an identity, the type and a string
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(msgData)
      }));
    }

    /// Create a No-ID Message out of an identity, the type and an int
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         int msgData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(msgData)
      }));
    }

    /// Create a No-ID Message out of an identity, the type and an event meta data
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const StoreObjPtr<EventMetaData>& evtMetaData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(evtMetaData)
      }));
    }

    /// Create a No-ID Message out of the type and another zmq message
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         zmq::message_t msgData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        std::move(msgData)
      }));
    }

    /// Create a No-ID Message out of the type, another zmq message and an additional message
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         zmq::message_t msgData,
                                                         zmq::message_t additionalData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        std::move(msgData), std::move(additionalData)
      }));
    }

    /// Create a No-ID Message out of an identity, the type and another zmq message
    static std::unique_ptr<ZMQNoIdMessage> createMessage(zmq::message_t msgType,
                                                         zmq::message_t msgData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        std::move(msgType),
        std::move(msgData)
      }));
    }

    /// Create a No-ID Message out of an identity, the type and an event message
    static std::unique_ptr<ZMQNoIdMessage>
    createMessage(const c_MessageTypes msgType, const std::unique_ptr<EvtMessage>& eventMessage)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(eventMessage)
      }));
    }

    static std::unique_ptr<ZMQNoIdMessage>
    createMessage(const c_MessageTypes msgType, const std::unique_ptr<EvtMessage>& eventMessage,
                  zmq::message_t additionalData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(msgType),
        ZMQMessageHelper::createZMQMessage(
          eventMessage),
        std::move(additionalData)
      }));
    }

    /// Create a No-ID Message out of an ID message
    static std::unique_ptr<ZMQNoIdMessage> stripIdentity(std::unique_ptr<ZMQIdMessage> message)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage({
        ZMQMessageHelper::createZMQMessage(std::move(message->getMessagePart<1>())),
        ZMQMessageHelper::createZMQMessage(std::move(message->getMessagePart<2>())),
        ZMQMessageHelper::createZMQMessage(std::move(message->getMessagePart<3>()))
      }));
    }


    /// Create a message of the given type by receiving a message from the socket.
    template<class AMessage>
    static std::unique_ptr<AMessage> fromSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      auto newMessage = std::unique_ptr<AMessage>(new AMessage());
      auto& messageParts = newMessage->getMessageParts();
      for (unsigned int i = 0; i < AMessage::c_messageParts; i++) {
        B2ASSERT("The next part does not belong to the same message",
                 socket->getsockopt<int>(ZMQ_RCVMORE) == 1 or i == 0);
        socket->recv(&messageParts[i]);
      }
      B2ASSERT("There should not be more than the retrieved parts", socket->getsockopt<int>(ZMQ_RCVMORE) == 0);
      return newMessage;
    }
  };
}