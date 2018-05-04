#pragma once

#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <memory>
#include <string>

namespace Belle2 {
  class ZMQMessageFactory {
  public:

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
                                                       const c_MessageTypes msgType,
                                                       const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQIdMessage>(new ZMQIdMessage(msgIdentity, msgType, eventMessage));
    }

    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::string& msgData = "")
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, msgData));
    }


    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, eventMessage));
    }

    template <class AMessage>
    static std::unique_ptr<AMessage> fromSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      auto newMessage = std::unique_ptr<AMessage>();
      auto& messageParts = newMessage->getMessageParts();
      for (int i = 0; i < AMessage::c_messageParts; i++) {
        socket->recv(&messageParts[i]);
      }
      return newMessage;
    }
  };
}