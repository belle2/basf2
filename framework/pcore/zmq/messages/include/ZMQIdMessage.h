#pragma once

#include <framework/pcore/zmq/messages/ZMQModuleMessage.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

#include <zmq.hpp>
#include <memory>

namespace Belle2 {

  class ZMQIdMessage : public ZMQModuleMessage<3> {
  public:
    /// The if the message is of a given type
    bool isMessage(const c_MessageTypes isType) const
    {
      const auto& type = getMessagePartAsString<c_type>();
      return type.size() == 1 and type[0] == static_cast<char>(isType);
    }

    /// Is the data part empty?
    bool isEmpty() const
    {
      return getMessagePart<c_data>().size() == 0;
    }

    /// Get the identity part
    std::string getIdentity() const
    {
      return getMessagePartAsString<c_identity>();
    }

  private:
    /// Where the identity is stored
    static const unsigned int c_identity = 0;
    /// Where the type of the message is stored
    static const unsigned int c_type = 1;
    /// Where the data is stored
    static const unsigned int c_data = 2;

    /// Copy the constructor from the base class
    using ZMQModuleMessage::ZMQModuleMessage;
  };
}
