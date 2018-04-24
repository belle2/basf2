#pragma once

#include <zmq.hpp>
#include <framework/pcore/EvtMessage.h>
#include <memory>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/zmq/modules/ZMQDefinitions.h>
#include <framework/pcore/zmq/sockets/ZMQSocket.h>

namespace Belle2 {

  class ZMQIdMessage {
  public:
    static const unsigned int c_msgparts = 3;

    ZMQIdMessage(const ZMQIdMessage&) = delete; // you cant copy a message


    static zmq::message_t createZMQMessage(const char& c)
    {
      return createZMQMessage(std::string(1, c));
    }

    static zmq::message_t createZMQMessage(const std::string& s)
    {
      return zmq::message_t(s.c_str(), s.length());
    }

    static zmq::message_t createZMQMessage(const std::unique_ptr<EvtMessage>& evtMessage)
    {
      return zmq::message_t(evtMessage->buffer(), evtMessage->size());
    }

    /*
        static zmq::message_t createZMQMessage(const std::unique_ptr<DataStoreStreamer>& streamer)
        {
          return zmq::message_t(streamer);
        }
    */

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
      std::array<zmq::message_t, c_msgparts>& messageArray = newMessage->getMessageParts();
      int num_revents = zmq::poll(&items[0], 1, timeout);
      if (items[0].revents & ZMQ_POLLIN) {
        for (int i = 0; i < c_msgparts; i++) {
          socket->recv(&messageArray[i]);
          /*
          int more;
          size_t more_size = sizeof(more);
          disSocket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
          if (!more) { break; }*/
        }

      }
      return newMessage;
    }


    void toSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      int socket_type;
      //size_t socket_type_size = sizeof(socket_type);
      //socket->getsockopt(ZMQ_TYPE, &socket_type, &socket_type_size);
      for (int i = 0; i < c_msgparts; i++) {
        if (i == c_msgparts - 1) {
          socket->send(m_messageParts[i]);
        } else {
          socket->send(m_messageParts[i], ZMQ_SNDMORE);
        }
      }
    }


    /*
    std::string getSenderID()
    {
      return std::string(static_cast<char*>(m_zmqMessage[c_identity].data()), m_zmqMessage[c_identity].size());
    }*/


    bool isMessage(const c_MessageTypes isType)
    {
      const char* type = static_cast<const char*>(m_messageParts[c_type].data());
      return type[0] == static_cast<char>(isType);
    }

    bool isEmpty()
    {
      return m_messageParts[c_data].size() == 0;
    }

  private:
    ZMQIdMessage() = default; // just allowed create messages with createMessage() or fromSocket

    ZMQIdMessage(const std::string& msgIdentity, const c_MessageTypes msgType, const std::string& msgData) :
      m_messageParts(
    {
      createZMQMessage(msgIdentity),
                       createZMQMessage(static_cast<char>(msgType)),
                       createZMQMessage(msgData)
    })
    {
    }

    /*
    ZMQIdMessage(const std::string& msgIdentity, const c_MessageTypes msgType, const std::unique_ptr<DataStoreStreamer>& streamer) :
          m_messageParts({createZMQMessage(msgIdentity),
                          createZMQMessage(static_cast<char>(msgType)),
                          createZMQMessage(streamer)})
    {
    }
    */
    ZMQIdMessage(const std::string& msgIdentity, const c_MessageTypes msgType, const std::unique_ptr<EvtMessage>& eventMessage) :
      m_messageParts(
    {
      createZMQMessage(msgIdentity),
                       createZMQMessage(static_cast<char>(msgType)),
                       createZMQMessage(eventMessage)
    })
    {
    }


    std::array<zmq::message_t, ZMQIdMessage::c_msgparts>& getMessageParts()
    {
      return m_messageParts;
    };

    const unsigned int c_identity = 0;
    const unsigned int c_type = 1;
    const unsigned int c_data = 2;


    std::array<zmq::message_t, c_msgparts> m_messageParts;
  };

}
