#pragma once

#include <framework/pcore/zmq/messages/ZMQModuleMessage.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/sockets/ZMQSocket.h>

#include <zmq.hpp>
#include <memory>

namespace Belle2 {

  class ZMQIdMessage : public ZMQModuleMessage<3> {
  public:
    static std::unique_ptr<ZMQIdMessage> createMessage(const std::string& msgIdentity,
                                                       const c_MessageTypes msgType,
                                                       const std::string& msgData)
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

    static std::unique_ptr<ZMQIdMessage> fromSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      auto newMessage = std::unique_ptr<ZMQIdMessage>();

      zmq::pollitem_t items[] = {
        {static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0}
      };
      int timeout = 1000;
      std::array<zmq::message_t, c_messageParts>& messageArray = newMessage->getMessageParts();
      int num_revents = zmq::poll(&items[0], 1, timeout);
      if (items[0].revents & ZMQ_POLLIN) {
        for (int i = 0; i < c_messageParts; i++) {
          socket->recv(&messageArray[i]);
        }
      }
      return newMessage;
    }

    /// The if the message is of a given type
    bool isMessage(const c_MessageTypes isType) const
    {
      const auto& type = getMessagePartAsString<c_type>();
      return type.size() == 1 and type[0] == static_cast<char>(isType);
    }

    /// Is the data part empty?
    bool isEmpty() const
    {
      return getMessagePart<c_data>().size() == 0;
    }

    /// Get the identity part
    std::string getIdentity() const
    {
      return getMessagePartAsString<c_identity>();
    }

  private:
    /// Where the identity is stored
    static const unsigned int c_identity = 0;
    /// Where the type of the message is stored
    static const unsigned int c_type = 1;
    /// Where the data is stored
    static const unsigned int c_data = 2;

    /// Copy the constructor from the base class
    using ZMQModuleMessage::ZMQModuleMessage;
  };
}
