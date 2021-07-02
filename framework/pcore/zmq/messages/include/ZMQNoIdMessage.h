/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/messages/ZMQModuleMessage.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>

#include <zmq.hpp>

namespace Belle2 {
  /// A message without an associated socket ID. Is used in most cases.
  class ZMQNoIdMessage : public ZMQModuleMessage<3> {
    friend class ZMQMessageFactory;

  public:
    /// Where the type is stored
    static constexpr const unsigned int c_type = 0;
    /// Where the data is stored
    static constexpr const unsigned int c_data = 1;
    /// Where the additional data is stored
    static constexpr const unsigned int c_additionalData = 2;

    /// Get the data as string
    std::string getData() const;

    /// Get the data part
    zmq::message_t& getDataMessage()
    {
      return getMessagePart<c_data>();
    }

    /// Get the additional data part
    zmq::message_t& getAdditionalDataMessage()
    {
      return getMessagePart<c_additionalData>();
    }

    /// The if the message is of a given type
    bool isMessage(EMessageTypes isType) const;

  private:
    /// Copy the constructors
    using ZMQModuleMessage::ZMQModuleMessage;
  };
}
