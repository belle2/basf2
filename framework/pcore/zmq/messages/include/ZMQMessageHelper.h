/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/pcore/zmq/utils/EventMetaDataSerialization.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <zmq.hpp>
#include <string>
#include <memory>

namespace Belle2 {

  /// Internal helper for creating ZMQ messages (should not be used otherwise).
  class ZMQMessageHelper {
  public:
    /// Just pass a zmq message
    static zmq::message_t createZMQMessage(zmq::message_t message)
    {
      return message;
    }

    /// Create a message out of a type by casting to a char
    static zmq::message_t createZMQMessage(const EMessageTypes& c)
    {
      return createZMQMessage(std::string(1, static_cast<char>(c)));
    }

    /// Create a message out of a string
    static zmq::message_t createZMQMessage(const std::string& s)
    {
      return zmq::message_t(s.c_str(), s.length());
    }

    /// Create a message out of an int by casting to a string
    static zmq::message_t createZMQMessage(const int i)
    {
      return createZMQMessage(std::to_string(i));
    }

    /// Create a message out of an event meta data by serialization.
    static zmq::message_t createZMQMessage(const StoreObjPtr<EventMetaData>& evtMetaData)
    {
      const auto& message = EventMetaDataSerialization::serialize(*evtMetaData);
      return zmq::message_t(message.c_str(), message.length());
    }

    /// Create a message out of an event message.
    static zmq::message_t createZMQMessage(const std::unique_ptr<EvtMessage>& evtMessage)
    {
      // TODO: here are the messages copied
      return zmq::message_t(evtMessage->buffer(), evtMessage->size());
    }

  };
}
