#pragma once

#include <framework/pcore/zeromq/ZeroMQMessage.h>

namespace Belle2 {

  class ZeroMQRoutedMessage {
  public:
    ZeroMQRoutedMessage() = delete;
    ZeroMQRoutedMessage(const ZeroMQRoutedMessage&) = delete;

    static std::unique_ptr<ZeroMQRoutedMessage> fromSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      // TODO: Do we need the zmq::message here?
      zmq::message_t identityMessage;
      socket->recv(&identityMessage);

      const std::string& workerIDAsString = std::string(static_cast<char*>(identityMessage.data()),
                                                        identityMessage.size());

      unsigned int workerID = std::stoul(workerIDAsString);
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::fromSocket(socket), workerID));
    }

    static std::unique_ptr<ZeroMQRoutedMessage> fromDataStore(const std::unique_ptr<DataStoreStreamer>& streamer,
                                                              unsigned int workerID)
    {
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::fromDataStore(streamer), workerID));
    }

    static std::unique_ptr<ZeroMQRoutedMessage> fromEventMessage(EvtMessage& message,
        unsigned int workerID)
    {
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::fromEventMessage(message), workerID));
    }

    static std::unique_ptr<ZeroMQRoutedMessage> createEndMessage(unsigned int workerID)
    {
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::createEndMessage(), workerID));
    }

    static std::unique_ptr<ZeroMQRoutedMessage> createReadyMessage(unsigned int workerID)
    {
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::createReadyMessage(), workerID));
    }

    static std::unique_ptr<ZeroMQRoutedMessage> createEmptyMessage()
    {
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::createEmptyMessage(), 0));
    }

    static std::unique_ptr<ZeroMQRoutedMessage> createHelloMessage()
    {
      return std::unique_ptr<ZeroMQRoutedMessage>(new ZeroMQRoutedMessage(ZeroMQMessage::createHelloMessage(), 0));
    }

    void toSocket(const std::unique_ptr<zmq::socket_t>& socket)
    {
      // TODO: Better use a zmq::message here or not?
      const std::string& workerIDAsString = getWorkerIDAsString();
      socket->send(workerIDAsString.data(), workerIDAsString.size(), ZMQ_SNDMORE);
      m_message->toSocket(socket);
    }

    unsigned int getWorkerID() const
    {
      return m_workerID;
    }

    bool isReadyMessage() const
    {
      return m_message->isReadyMessage();
    }

    bool isHelloMessage() const
    {
      return m_message->isHelloMessage();
    }

    void setWorkerId(unsigned int workerID)
    {
      m_workerID = workerID;
    }

    bool empty() const
    {
      return m_message->empty();
    }

  private:
    ZeroMQRoutedMessage(std::unique_ptr<ZeroMQMessage>&& message, unsigned int workerID) :
      m_message(std::move(message)), m_workerID(workerID) {}

    std::string getWorkerIDAsString() const
    {
      return std::to_string(m_workerID);
    }

    std::unique_ptr<ZeroMQMessage> m_message;
    unsigned int m_workerID;
  };
}