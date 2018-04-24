#pragma once

#include <zmq.hpp>
#include <memory>
#include <sstream>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>
#include <framework/core/RandomNumbers.h>
#include <framework/pcore/SeqFile.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/sockets/ZMQSocket.h>
#include <framework/logging/LogMethod.h>


namespace Belle2 {

  class ZMQNoIdMessage {
  public:
    static const unsigned int c_msgparts = 2;
    ZMQNoIdMessage(const ZMQNoIdMessage&) = delete; // you cant copy a message

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


// ################################### from Socket #######################################

    static std::unique_ptr<ZMQNoIdMessage> fromSocket(std::unique_ptr<ZMQSocket>& socket)
    {
      auto newMessage = std::unique_ptr<ZMQNoIdMessage>();

      zmq::pollitem_t items[] = {
        {static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0}
      };
      int timeout = 1000;
      std::array<zmq::message_t, c_msgparts>& messageArray = newMessage->getMessageParts();
      int num_revents = zmq::poll(&items[0], 1, timeout);
      if (items[0].revents & ZMQ_POLLIN) {
        for (int i = 0; i < c_msgparts; i++) {
          socket->recv(&messageArray[i]);
        }

      }
      return newMessage;
    }


// ################################### to Socket #######################################

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


    bool isMessage(const c_MessageTypes isType)
    {
      const char* type = charPtrToMsgPart(c_type);
      return type[0] == static_cast<char>(isType);
    }


// ################################### to Datastore #######################################

    void toDataStore(const std::unique_ptr<DataStoreStreamer>& streamer,
                     const StoreObjPtr<RandomGenerator>& randomGenerator)
    {
      //B2DEBUG(100, "Write back to datastore.");

      //B2ASSERT("The message can not be an end/ready message for streaming!",
      //         isMessage(c_MessageTypes::c_eventMessage));

      EvtMessage eventMessage(charPtrToMsgPart(c_data));
      streamer->restoreDataStore(&eventMessage);
    }


    void toSeqFile(const std::unique_ptr<SeqFile>& seqFile)
    {
      EvtMessage eventMessage(charPtrToMsgPart(c_data));
      seqFile->write(eventMessage.buffer());
      //B2DEBUG(100, "Written back to file.");
    }


    std::string getData()
    {
      if (isMessage(c_MessageTypes::c_eventMessage)) {
        B2ERROR("Message is Event Message getData() is not allowed");
        return "";
      } else {
        return std::string(static_cast<char*>(m_messageParts[c_data].data()));
      }
    }

  private:
    ZMQNoIdMessage() = default; // just allowed create messages with createMessage() or fromSocket

    ZMQNoIdMessage(const c_MessageTypes msgType, const std::string& msgData) :
      m_messageParts(
    {
      createZMQMessage(static_cast<char>(msgType)),
                       createZMQMessage(msgData)
    })
    {
    }

    ZMQNoIdMessage(const c_MessageTypes msgType, const std::unique_ptr<EvtMessage>& eventMessage) :
      m_messageParts(
    {
      createZMQMessage(static_cast<char>(msgType)),
                       createZMQMessage(eventMessage)
    })
    {
    }


    const char* charPtrToMsgPart(const unsigned int part) const
    {
      return  static_cast<const char*>(m_messageParts[part].data());
    }

    char* charPtrToMsgPart(const unsigned int part)
    {
      return  static_cast<char*>(m_messageParts[part].data());
    }


    std::array<zmq::message_t, ZMQNoIdMessage::c_msgparts>& getMessageParts()
    {
      return m_messageParts;
    };


    const unsigned int c_type = 1;
    const unsigned int c_data = 2;

    std::array<zmq::message_t, c_msgparts> m_messageParts;

  };
}