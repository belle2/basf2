#pragma once

#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>
#include <framework/pcore/zmq/messages/EventMessageBuffer.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/logging/LogMethod.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <memory>
#include <string>
#include <zmq.hpp>

namespace Belle2 {
  class ZMQMessageFactory {
  public:

// ------------------------------------------- ID -----------------------------------------------------
    // Message for the TxSeqRootInputModule
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const std::unique_ptr<EvtMessage>& eventMessage)
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, c_MessageTypes::c_eventMessage, eventMessage));
    }


    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, msgType, msgData));
    }


    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const EventMessageBuffer& evtMsgBuffer)
    {
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, c_MessageTypes::c_eventMessage, evtMsgBuffer));
    }


    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, c_MessageTypes::c_eventMessage, eventMessage));
    }

// --------------------------------------- NO ID -----------------------------------------------------------
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, msgData));
    }


    static std::unique_ptr<ZMQNoIdMessage> createMessage(const EventMessageBuffer& evtMsgBuffer)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(c_MessageTypes::c_eventMessage, evtMsgBuffer));
    }


    static std::unique_ptr<ZMQNoIdMessage> createMessage(const UniqueEventId& evtId)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(c_MessageTypes::c_confirmMessage, evtId));
    }


    // TODO: do we still need?

    static std::unique_ptr<ZMQNoIdMessage> createMessage(const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(c_MessageTypes::c_eventMessage, eventMessage));
    }


    template <class AMessage>
    static std::unique_ptr<AMessage> fromSocket(std::unique_ptr<ZMQSocket>& socket, bool printMessage = false)
    {
      auto newMessage = std::unique_ptr<AMessage>(new AMessage());
      auto& messageParts = newMessage->getMessageParts();
      for (unsigned int i = 0; i < AMessage::c_messageParts; i++) {
        zmq::message_t message;
        socket->recv(&messageParts[i]);
        if (printMessage)
          B2RESULT("From " << std::string(static_cast<const char*>(messageParts[i].data()), messageParts[i].size()));
      }

      return newMessage;
    }
  };
}