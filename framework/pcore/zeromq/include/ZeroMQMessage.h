#pragma once

#include <framework/pcore/DataStoreStreamer.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/core/RandomNumbers.h>
#include <framework/pcore/SeqFile.h>
#include <memory.h>
#include <zmq.hpp>

namespace Belle2 {

  class ZeroMQMessage {
  public:
    enum class c_MessageTypes {
      c_eventMessage = 'e',
      c_endMessage = 't',
      c_readyMessage = 'r',
      c_helloMessage = 'h'
    };

    static std::unique_ptr<ZeroMQMessage> fromSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      B2DEBUG(100, "Reading from socket");
      zmq::message_t message;
      socket->recv(&message);
      B2DEBUG(100, "Received message with size " << message.size());
      return std::unique_ptr<ZeroMQMessage>(new ZeroMQMessage(std::move(message)));
    }

    static std::unique_ptr<ZeroMQMessage> fromEventMessage(EvtMessage& eventMessage)
    {
      // Size + 1 for first type bit
      zmq::message_t message(eventMessage.size() + 1);

      // Type bit
      const char type = static_cast<char>(c_MessageTypes::c_eventMessage);
      memcpy(message.data(), &type, 1);

      // TODO: Is copy necessary?
      memcpy(&(static_cast<char*>(message.data())[1]), eventMessage.buffer(), eventMessage.size());
      B2DEBUG(100, "Finished reading in from datastore with size " << eventMessage.size());
      return std::unique_ptr<ZeroMQMessage>(new ZeroMQMessage(std::move(message)));
    }

    static std::unique_ptr<ZeroMQMessage> fromDataStore(const std::unique_ptr<DataStoreStreamer>& streamer)
    {
      B2DEBUG(100, "Reading in from datastore");
      // TODO: Handle streaming of persistent objects correctly
      std::unique_ptr<EvtMessage> eventMessage(streamer->streamDataStore(true, true));
      return ZeroMQMessage::fromEventMessage(*eventMessage);
    }

    static std::unique_ptr<ZeroMQMessage> createEndMessage()
    {
      B2DEBUG(100, "Created end message");
      zmq::message_t message(1);
      const char type = static_cast<char>(c_MessageTypes::c_endMessage);
      memcpy(message.data(), &type, 1);
      return std::unique_ptr<ZeroMQMessage>(new ZeroMQMessage(std::move(message)));
    }

    static std::unique_ptr<ZeroMQMessage> createReadyMessage()
    {
      B2DEBUG(100, "Created ready message");
      zmq::message_t message(1);
      const char type = static_cast<char>(c_MessageTypes::c_readyMessage);
      memcpy(message.data(), &type, 1);
      return std::unique_ptr<ZeroMQMessage>(new ZeroMQMessage(std::move(message)));
    }

    static std::unique_ptr<ZeroMQMessage> createHelloMessage()
    {
      B2DEBUG(100, "Created hello message");
      zmq::message_t message(1);
      const char type = static_cast<char>(c_MessageTypes::c_helloMessage);
      memcpy(message.data(), &type, 1);
      return std::unique_ptr<ZeroMQMessage>(new ZeroMQMessage(std::move(message)));
    }

    static std::unique_ptr<ZeroMQMessage> createEmptyMessage()
    {
      zmq::message_t message(0);
      return std::unique_ptr<ZeroMQMessage>(new ZeroMQMessage(std::move(message)));
    }

    ZeroMQMessage() = delete;
    ZeroMQMessage(const ZeroMQMessage&) = delete;


    bool isEndMessage() const
    {
      B2DEBUG(100, getMessageTypeBit());
      return getMessageTypeBit() == static_cast<char>(c_MessageTypes::c_endMessage);
    }

    bool isReadyMessage() const
    {
      B2DEBUG(100, getMessageTypeBit());
      return getMessageTypeBit() == static_cast<char>(c_MessageTypes::c_readyMessage);
    }

    bool isEventMessage() const
    {
      B2DEBUG(100, getMessageTypeBit());
      return getMessageTypeBit() == static_cast<char>(c_MessageTypes::c_eventMessage);
    }

    bool isHelloMessage() const
    {
      B2DEBUG(100, getMessageTypeBit());
      return getMessageTypeBit() == static_cast<char>(c_MessageTypes::c_helloMessage);
    }

    bool empty() const
    {
      return m_message.size() == 0;
    }


    void toDataStore(const std::unique_ptr<DataStoreStreamer>& streamer,
                     const StoreObjPtr<RandomGenerator>& randomGenerator)
    {
      B2DEBUG(100, "Write back to datastore.");

      B2ASSERT("The message can not be an end/ready message for streaming!",
               isEventMessage());

      char* pointerToCharData = getPointerToCharData();
      EvtMessage eventMessage(&(pointerToCharData[1]));
      streamer->restoreDataStore(&eventMessage);

      // Restore the event dependent random number object from Datastore
      if (randomGenerator.isValid()) {
        RandomNumbers::getEventRandomGenerator() = *randomGenerator;
      }
    }

    void toSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      B2DEBUG(100, "Write back to socket.");
      const int rc = socket->send(m_message, ZMQ_NOBLOCK);
      B2ASSERT("Sending failed", rc);
    }

    void toSeqFile(const std::unique_ptr<SeqFile>& seqFile)
    {
      B2DEBUG(100, "Write to file.");
      char* pointerToCharData = getPointerToCharData();
      EvtMessage eventMessage(&(pointerToCharData[1]));
      seqFile->write(eventMessage.buffer());
      B2DEBUG(100, "Written back to file.");
    }

  private:
    zmq::message_t m_message;

    ZeroMQMessage(zmq::message_t&& message) : m_message(std::move(message)) {}

    char getMessageTypeBit() const
    {
      return getPointerToCharData()[0];
    }

    const char* getPointerToCharData() const
    {
      return static_cast<const char*>(m_message.data());
    }

    char* getPointerToCharData()
    {
      return static_cast<char*>(m_message.data());
    }
  };
}