#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>

using namespace Belle2;

/// Get the data as string
std::string ZMQNoIdMessage::getData() const
{
  B2ASSERT("The message is an event message",
           not isMessage(c_MessageTypes::c_eventMessage));
  return getMessagePartAsString<c_data>();
}

/// The if the message is of a given type
bool ZMQNoIdMessage::isMessage(const c_MessageTypes isType) const
{
  const auto& type = getMessagePartAsString<c_type>();
  return type.size() == 1 and type[0] == static_cast<char>(isType);
}