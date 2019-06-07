/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/messages/ZMQModuleMessage.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>

#include <zmq.hpp>
#include <memory>

namespace Belle2 {
  /// A message with the socket ID. Needed in input<->worker communication.
  class ZMQIdMessage : public ZMQModuleMessage<4> {
    friend class ZMQMessageFactory;

  public:
    /// Where the identity is stored
    static constexpr const unsigned int c_identity = 0;
    /// Where the type of the message is stored
    static constexpr const unsigned int c_type = 1;
    /// Where the data is stored
    static constexpr const unsigned int c_data = 2;
    /// Where the additional data is stored
    static constexpr const unsigned int c_additionalData = 3;

    /// The if the message is of a given type
    bool isMessage(const EMessageTypes isType) const
    {
      const auto& type = getMessagePartAsString<c_type>();
      return type.size() == 1 and type[0] == static_cast<char>(isType);
    }

    /// Get the identity part
    std::string getIdentity() const
    {
      return getMessagePartAsString<c_identity>();
    }

    zmq::message_t& getDataMessage()
    {
      return getMessagePart<c_data>();
    }

    zmq::message_t& getAdditionalDataMessage()
    {
      return getMessagePart<c_additionalData>();
    }

  private:
    /// Copy the constructor from the base class
    using ZMQModuleMessage::ZMQModuleMessage;
  };
}
