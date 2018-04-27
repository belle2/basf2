#pragma once

#include <framework/pcore/zmq/messages/ZMQModuleMessage.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/core/RandomNumbers.h>
#include <framework/pcore/SeqFile.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/sockets/ZMQSocket.h>
#include <framework/logging/LogMethod.h>

#include <zmq.hpp>
#include <memory>



namespace Belle2 {

  class ZMQNoIdMessage : public ZMQModuleMessage<2> {
  public:
    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::string& msgData)
    {
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, msgData));
    }

    static std::unique_ptr<ZMQNoIdMessage> createMessage(const c_MessageTypes msgType,
                                                         const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return std::unique_ptr<ZMQNoIdMessage>(new ZMQNoIdMessage(msgType, eventMessage));
    }

    static std::unique_ptr<ZMQNoIdMessage> fromSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      auto newMessage = std::unique_ptr<ZMQNoIdMessage>();

      zmq::pollitem_t items[] = {
        {static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0}
      };
      int timeout = 1000;
      MessageParts& messageArray = newMessage->getMessageParts();
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

    /// Write the data to the data store
    void toDataStore(const std::unique_ptr<DataStoreStreamer>& streamer,
                     const StoreObjPtr<RandomGenerator>& randomGenerator)
    {
      // TODO: include the random generator here
      B2ASSERT("The message can not be an end/ready message for streaming!",
               isMessage(c_MessageTypes::c_eventMessage));

      EvtMessage eventMessage(getMessagePartAsCharArray<c_data>());
      streamer->restoreDataStore(&eventMessage);
    }

    /// Write the data to a seq file
    void toSeqFile(const std::unique_ptr<SeqFile>& seqFile)
    {
      B2ASSERT("The message can not be an end/ready message for streaming!",
               isMessage(c_MessageTypes::c_eventMessage));

      EvtMessage eventMessage(getMessagePartAsCharArray<c_data>());
      seqFile->write(eventMessage.buffer());
    }

    /// Get the data as string
    std::string getData() const
    {
      B2ASSERT("The message is an event message",
               not isMessage(c_MessageTypes::c_eventMessage));
      return getMessagePartAsString<c_data>();
    }

  private:
    /// Copy the constructors
    using ZMQModuleMessage::ZMQModuleMessage;

    /// Where the type is stored
    static constexpr const unsigned int c_type = 0;
    /// Where the data is stored
    static constexpr const unsigned int c_data = 1;
  };
}