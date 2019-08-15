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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>

#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/logging/LogMethod.h>

#include <zmq.hpp>
#include <memory>



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
