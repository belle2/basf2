#pragma once

#include <framework/pcore/zmq/messages/ZMQMessageHelper.h>
#include <framework/logging/LogMethod.h>

#include <zmq.hpp>
#include <memory>
#include <array>

namespace Belle2 {

  template <unsigned int AMessageFrameNumber>
  class ZMQModuleMessage {
    friend class ZMQMessageFactory;

  public:
    /// The number of message parts this message carries
    static constexpr unsigned int c_messageParts = AMessageFrameNumber;

    /// The base class of the message parts
    using MessageParts = std::array<zmq::message_t, ZMQModuleMessage::c_messageParts>;

    /// Do not allow to copy a message
    ZMQModuleMessage(const ZMQModuleMessage&) = delete;

    /// Send the message to the given socket
    void toSocket(const std::unique_ptr<zmq::socket_t>& socket, bool printMessage = false)
    {
      for (unsigned int i = 0; i < c_messageParts - 1; i++) {
        if (printMessage) {
          B2RESULT(std::string(static_cast<const char*>(m_messageParts[i].data()), m_messageParts[i].size()));
        }
        socket->send(m_messageParts[i], ZMQ_SNDMORE);
      }
      socket->send(m_messageParts[c_messageParts - 1]);
      if (printMessage) {
        B2RESULT(std::string(static_cast<const char*>(m_messageParts[c_messageParts - 1].data()),
                             m_messageParts[c_messageParts - 1].size()));
      }
    }

  protected:
    template <class ...T>
    ZMQModuleMessage(const T& ... arguments) :
      m_messageParts( {ZMQMessageHelper::createZMQMessage(arguments)...})
    {
    }

    /// Get a reference to the message parts
    MessageParts& getMessageParts()
    {
      return m_messageParts;
    };

    /// Get a const reference to the message parts
    const MessageParts& getMessageParts() const
    {
      return m_messageParts;
    };

    /// Get the message part with the given index (const version)
    template <unsigned int index>
    const zmq::message_t& getMessagePart() const
    {
      return m_messageParts[index];
    }

    /// Get the message part with the given index as char* (const version)
    template <unsigned int index>
    const char* getMessagePartAsCharArray() const
    {
      const auto& messagePart = getMessagePart<index>();
      return static_cast<const char*>(messagePart.data());
    }

    /// Get the message part with the given index as string (const version)
    template <unsigned int index>
    std::string getMessagePartAsString() const
    {
      const auto& messagePart = getMessagePart<index>();
      return std::string(static_cast<const char*>(messagePart.data()), messagePart.size());
    }

    /// Get the message part with the given index
    template <unsigned int index>
    zmq::message_t& getMessagePart()
    {
      return m_messageParts[index];
    }

    /// Get the message part with the given index as char*
    template <unsigned int index>
    char* getMessagePartAsCharArray()
    {
      auto& messagePart = getMessagePart<index>();
      return static_cast<char*>(messagePart.data());
    }

  protected:
    /// Do not allow to create a new message from scratch publicly
    ZMQModuleMessage() = default;

  private:
    /// The content of this message as an array of zmq messages. Will be set during constructor or when coming from a socket.
    MessageParts m_messageParts;
  };
}
